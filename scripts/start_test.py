import argparse
import os
import subprocess
import time
import yaml
from tqdm import tqdm
from concurrent.futures import ThreadPoolExecutor

ssh_user = "root"
sar_cmd = "sar -o /var/log/sar_%HOST%_%TIMESTAMP%.log 1"



def start_remote_sar(host):
    timestamp = subprocess.check_output(["date", "+%s"]).decode().strip()
    cmd = sar_cmd.replace("%HOST%", host).replace("%TIMESTAMP%", timestamp)
    ssh_args = [
        "ssh", "-o", "StrictHostKeyChecking=no", 
        f"{ssh_user}@{host}",
        f"nohup {cmd} >/dev/null 2>&1 & echo $! > /tmp/sar_{timestamp}.pid"
    ]
    proc = subprocess.Popen(
        ssh_args,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )
    return proc

def stop_remote_sar(host):
    kill_cmd = "cat /tmp/sar_*.pid | xargs kill"
    subprocess.Popen(
        ["ssh", "-o", "StrictHostKeyChecking=no", f"{ssh_user}@{host}", kill_cmd],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )
    rm_cmd = "rm /tmp/sar*"
    subprocess.Popen(
        ["ssh", "-o", "StrictHostKeyChecking=no", f"{ssh_user}@{host}", rm_cmd],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )

def copy_sar_log(host, folder):
    scp_cmd = f"scp -o StrictHostKeyChecking=no {ssh_user}@{host}:/var/log/sar* {folder}"
    subprocess.run(scp_cmd, shell=True)
    rm_cmd = "rm -f /var/log/sar*"
    subprocess.Popen(
        ["ssh", "-o", "StrictHostKeyChecking=no", f"{ssh_user}@{host}", rm_cmd],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )

def start_sar(hosts):
    with ThreadPoolExecutor(max_workers=3) as executor:
        processes = executor.map(start_remote_sar, hosts)

def stop_sar(hosts, folder):
    for host in hosts:
        stop_remote_sar(host)
    for host in hosts:
        copy_sar_log(host, folder)

def drop_caches(hosts):
    for host in hosts:
        subprocess.run(["ssh", "-o", "StrictHostKeyChecking=no", f"{ssh_user}@{host}", "sync"])
        subprocess.run(["ssh", "-o", "StrictHostKeyChecking=no", f"{ssh_user}@{host}", "echo 3 > /proc/sys/vm/drop_caches"])

def check_var_path(config, path):
    current = config
    for key in path:
        if key in current:
            current = current[key]
        else:
            return False
    return True

def clean_sar(hosts):
    rm_log_cmd = "rm -f /var/log/sar*"
    rm_pid_cmd = "rm -f /tmp/sar*"
    kill_cmd = " ps -ef | grep sar | grep -v grep | awk '{print $2}' | xargs kill"
    for host in hosts:
        subprocess.Popen(
            ["ssh", "-o", "StrictHostKeyChecking=no", f"{ssh_user}@{host}", rm_log_cmd],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        subprocess.Popen(
            ["ssh", "-o", "StrictHostKeyChecking=no", f"{ssh_user}@{host}", rm_pid_cmd],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        subprocess.Popen(
            ["ssh", "-o", "StrictHostKeyChecking=no", f"{ssh_user}@{host}", kill_cmd],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )

def deal_with_yaml(args):
    file_path = args.config
    with open(file_path, 'r', encoding='utf-8') as f:
        test_config = yaml.safe_load(f)
    file_path = test_config["base_config"]
    with open(file_path, 'r', encoding='utf-8') as f:
        dlio_config = yaml.safe_load(f)
    servers = test_config["servers"]
    clients = test_config["clients"]
    hosts = servers + clients
    output_base = test_config["output_folder"]
    var_path = test_config["var"]["name"]
    if not check_var_path(dlio_config, var_path):
        print("var path error!")
        return
    rank = args.rank
    total_rank = rank * len(clients)
    with open("hosts.txt", 'w') as f:
        for client in clients:
            f.write(f"{client} slots={rank}\n")
    # before start clean sar
    clean_sar(hosts)
    for i in tqdm(range(test_config["var"]["start_val"], test_config["var"]["end_val"], test_config["var"]["step"])):
        # prepare output_folder
        sub_folder = f"rank_{rank}_" + "_".join(var_path)
        sub_folder = sub_folder + f"_{i}" 
        output_folder = os.path.join(output_base, sub_folder)
        os.makedirs(output_folder, exist_ok=True)
        # prepare run_config
        run_config = dlio_config
        current = run_config
        for key in var_path[:-1]:
            current = current[key]
        current[var_path[-1]] = i
        run_config["output"]["folder"] = output_folder
        with open("run.yaml", 'w', encoding='utf-8') as f:
            yaml.safe_dump(
                run_config, f,
                allow_unicode=True,
                sort_keys=False,
                indent=2
            )
        # drop cache
        drop_caches(hosts)
        # start sar
        start_sar(hosts)
        time.sleep(60)
        # start benchmark
        mpi_cmd=f"mpirun --allow-run-as-root -np {total_rank} --hostfile hosts.txt /root/dlio-perf/bin/dlio_perf --config run.yaml"
        subprocess.run(mpi_cmd, shell=True)
        # stop sar
        stop_sar(hosts, output_folder)
    rm_cmd = "rm run.yaml"
    subprocess.run(rm_cmd, shell=True)
    rm_cmd = "rm hosts.txt"
    subprocess.run(rm_cmd, shell=True)

def deal_with_rank(args):
    file_path = args.config
    with open(file_path, 'r', encoding='utf-8') as f:
        test_config = yaml.safe_load(f)
    file_path = test_config["base_config"]
    with open(file_path, 'r', encoding='utf-8') as f:
        dlio_config = yaml.safe_load(f)
    servers = test_config["servers"]
    clients = test_config["clients"]
    hosts = servers + clients
    output_base = test_config["output_folder"]
    for i in tqdm(range(test_config["var"]["start_val"], test_config["var"]["end_val"], test_config["var"]["step"])):
        # calculate rank
        rank = i
        total_rank = rank * len(clients)
        # prepare hosts.txt
        with open("hosts.txt", 'w') as f:
            for client in clients:
                f.write(f"{client} slots={rank}\n")
        # prepare output_folder
        sub_folder = f"client_{len(clients)}_rank"
        sub_folder = sub_folder + f"_{i}" 
        output_folder = os.path.join(output_base, sub_folder)
        os.makedirs(output_folder, exist_ok=True)
        # prepare run_config
        run_config = dlio_config
        run_config["output"]["folder"] = output_folder
        with open("run.yaml", 'w', encoding='utf-8') as f:
            yaml.safe_dump(
                run_config, f,
                allow_unicode=True,
                sort_keys=False,
                indent=2
            )
        # drop cache
        drop_caches(hosts)
        # start sar
        start_sar(hosts)
        time.sleep(60)
        # start benchmark
        mpi_cmd=f"mpirun --allow-run-as-root -np {total_rank} --hostfile hosts.txt /root/dlio-perf/bin/dlio_perf --config run.yaml"
        subprocess.run(mpi_cmd, shell=True)
        # stop sar
        stop_sar(hosts, output_folder)
    rm_cmd = "rm run.yaml"
    subprocess.run(rm_cmd, shell=True)
    rm_cmd = "rm hosts.txt"
    subprocess.run(rm_cmd, shell=True)

        

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--config', required=True, help='test config')
    parser.add_argument('-r', '--rank', help='mpi rank number. if not defined, rank is var')
    args = parser.parse_args()
    if args.rank:
        deal_with_yaml(args)
    else:
        deal_with_rank(args)