import argparse
import os
import subprocess
import time
import yaml
from tqdm import tqdm
from concurrent.futures import ThreadPoolExecutor

hosts = ["10.118.0.161"]
ssh_user = "root"
sar_cmd = "sar -o /var/log/sar_%HOST%_%TIMESTAMP%.log 1"



def start_remote_sar(host):
    timestamp = subprocess.check_output(["date", "+%s"]).decode().strip()
    cmd = sar_cmd.replace("%HOST%", host).replace("%TIMESTAMP%", timestamp)
    ssh_args = [
        "ssh", 
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
    kill_cmd = (
        "pid=$(cat /tmp/sar_*.pid);"
        "kill -9 $pid && rm /tmp/sar_*.pid || echo 'not running'"
    )
    subprocess.Popen(
        ["ssh", f"{ssh_user}@{host}", kill_cmd],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )

def copy_sar_log(host, folder):
    scp_cmd = f"scp {ssh_user}@{host}:/var/log/sar* {folder}"
    subprocess.run(scp_cmd, shell=True)
    rm_cmd = "rm -f /var/log/sar*"
    subprocess.Popen(
        ["ssh", f"{ssh_user}@{host}", rm_cmd],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )

def start_sar(hosts):
    timestamp = subprocess.check_output(["date", "+%s"]).decode().strip()
    cmd = sar_cmd.replace("%HOST%", "client").replace("%TIMESTAMP%", timestamp)
    cmd = f"nohup {cmd} >/dev/null 2>&1 & echo $! > /tmp/sar_{timestamp}.pid"
    subprocess.run(cmd, shell=True)
    with ThreadPoolExecutor(max_workers=3) as executor:
        processes = executor.map(start_remote_sar, hosts)

def stop_sar(hosts, folder):
    cmd = (
        "pid=$(cat /tmp/sar_*.pid);"
        "kill -9 $pid && rm /tmp/sar_*.pid || echo 'not running'"
    )
    subprocess.run(cmd, shell=True)
    cp_cmd = f"cp /var/log/sar* {folder}"
    subprocess.run(cp_cmd, shell=True)
    rm_cmd = f"rm /var/log/sar*"
    subprocess.run(rm_cmd, shell=True)
    for host in hosts:
        stop_remote_sar(host)
        copy_sar_log(host, folder)

def deal_with_yaml(args):
    file_path = args.config
    with open(file_path, 'r', encoding='utf-8') as f:
        test_config = yaml.safe_load(f)
    file_path = test_config["base_config"]
    with open(file_path, 'r', encoding='utf-8') as f:
        dlio_config = yaml.safe_load(f)
    servers = test_config["servers"]
    output_base = test_config["output_folder"]
    var_path = test_config["var"]["name"]
    rank = args.rank
    for i in tqdm(range(test_config["var"]["start_val"], test_config["var"]["end_val"], test_config["var"]["step"])):
        # prepare output_folder
        sub_folder = "_".join(var_path)
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
        subprocess.run(["sync"])
        subprocess.run("echo 3 > /proc/sys/vm/drop_caches", shell=True)
        for host in servers:
            subprocess.run(["ssh", f"{ssh_user}@{host}", "sync"])
            subprocess.run(["ssh", f"{ssh_user}@{host}", "echo 3 > /proc/sys/vm/drop_caches"])
        # start sar
        start_sar(servers)
        time.sleep(3)
        # start benchmark
        mpi_cmd=f"mpirun --allow-run-as-root -n {rank} /root/dlio-perf/bin/dlio_perf --config run.yaml"
        subprocess.run(mpi_cmd, shell=True)
        # stop sar
        stop_sar(servers, output_folder)
    rm_cmd = "rm run.yaml"
    subprocess.run(rm_cmd, shell=True)

def deal_with_rank(args):
    pass
        

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--config', required=True, help='test config')
    parser.add_argument('-r', '--rank', help='mpi rank number. if not defined, rank is var')
    args = parser.parse_args()
    if args.rank:
        deal_with_yaml(args)
    else:
        deal_with_rank(args)