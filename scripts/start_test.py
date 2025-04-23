import argparse
import os
import subprocess
import time
import yaml
from tqdm import tqdm
from concurrent.futures import ThreadPoolExecutor

ssh_user = "root"


def start_remote_sar(host):
    ssh_args = [
        "ssh", "-o", "StrictHostKeyChecking=no", 
        f"{ssh_user}@{host}",
        f"/tmp/stat_tools/run_stat.sh &"
    ]
    proc = subprocess.Popen(
        ssh_args,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )
    return proc

def stop_remote_sar(host):
    kill_cmd = "/tmp/stat_tools/stop_stat.sh"
    subprocess.Popen(
        ["ssh", "-o", "StrictHostKeyChecking=no", f"{ssh_user}@{host}", kill_cmd],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL
    )

def copy_sar_log(host, folder):
    scp_cmd = f"scp -o StrictHostKeyChecking=no {ssh_user}@{host}:/tmp/stat_tools/cpu {folder}/{host}_cpu"
    subprocess.run(scp_cmd, shell=True)
    scp_cmd = f"scp -o StrictHostKeyChecking=no {ssh_user}@{host}:/tmp/stat_tools/disk {folder}/{host}_disk"
    subprocess.run(scp_cmd, shell=True)
    scp_cmd = f"scp -o StrictHostKeyChecking=no {ssh_user}@{host}:/tmp/stat_tools/network {folder}/{host}_network"
    subprocess.run(scp_cmd, shell=True)

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

def rm_ck(folder):
    rm_cmd = f"rm -f {folder}/checkpoint*"
    subprocess.run(rm_cmd, shell=True)

def copy_bin(hosts):
    for host in hosts:
        mkdir_cmd = f"ssh -o StrictHostKeyChecking=no {ssh_user}@{host} mkdir -p /root/dlio-perf/bin"
        scp_cmd = f"scp -o StrictHostKeyChecking=no /root/dlio-perf/bin/dlio_perf {ssh_user}@{host}:/tmp"
        chmod_cmd = f"ssh -o StrictHostKeyChecking=no {ssh_user}@{host} chmod +777 /tmp/dlio_perf"
        subprocess.run(mkdir_cmd, shell=True)
        subprocess.run(scp_cmd, shell=True)
        subprocess.run(chmod_cmd, shell=True)

def copy_yaml(hosts):
    for host in hosts:
        scp_cmd = f"scp -o StrictHostKeyChecking=no /tmp/run.yaml {ssh_user}@{host}:/tmp/run.yaml"
        subprocess.run(scp_cmd, shell=True)

def deal_with_yaml(args):
    file_path = args.config
    with open(file_path, 'r', encoding='utf-8') as f:
        test_config = yaml.safe_load(f)
    file_path = test_config["base_config"]
    with open(file_path, 'r', encoding='utf-8') as f:
        dlio_config = yaml.safe_load(f)
    servers = test_config["servers"]
    clients = test_config["clients"]
    hosts = servers
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
    os.makedirs(output_base, exist_ok=True)
    var_config_file = os.path.join(output_base, "var.yaml")
    base_config_file = os.path.join(output_base, "base.yaml")
    with open(var_config_file, 'w', encoding='utf-8') as f:
            yaml.safe_dump(
                test_config, f,
                allow_unicode=True,
                sort_keys=False,
                indent=2
            )
    with open(base_config_file, 'w', encoding='utf-8') as f:
            yaml.safe_dump(
                dlio_config, f,
                allow_unicode=True,
                sort_keys=False,
                indent=2
            )
    # copy dlio_perf to client
    copy_bin(clients)
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
        with open("/tmp/run.yaml", 'w', encoding='utf-8') as f:
            yaml.safe_dump(
                run_config, f,
                allow_unicode=True,
                sort_keys=False,
                indent=2
            )
        copy_yaml(clients)
        # drop cache
        print("drop cache")
        drop_caches(hosts)
        drop_caches(clients)
        print("start sar")
        # start sar
        start_sar(hosts)
        time.sleep(60)
        # start benchmark
        print("start benchmark")
        mpi_cmd=f"mpirun -mca btl_tcp_if_include 10.118.0.0/24 --allow-run-as-root -np {total_rank} --hostfile hosts.txt /root/dlio-perf/bin/dlio_perf --config /tmp/run.yaml"
        subprocess.run(mpi_cmd, shell=True)
        # stop sar
        stop_sar(hosts, output_folder)
        rm_ck(dlio_config["checkpoint"]["checkpoint_folder"])
        time.sleep(60)
    rm_cmd = "rm /tmp/run.yaml"
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
    os.makedirs(output_base, exist_ok=True)
    subprocess.run(f"chmod +777 {output_base}", shell=True)
    var_config_file = os.path.join(output_base, "var.yaml")
    base_config_file = os.path.join(output_base, "base.yaml")
    with open(var_config_file, 'w', encoding='utf-8') as f:
            yaml.safe_dump(
                test_config, f,
                allow_unicode=True,
                sort_keys=False,
                indent=2
            )
    with open(base_config_file, 'w', encoding='utf-8') as f:
            yaml.safe_dump(
                dlio_config, f,
                allow_unicode=True,
                sort_keys=False,
                indent=2
            )
    # copy dlio_perf to client
    copy_bin(clients)
    for i in tqdm(range(test_config["var"]["start_val"], test_config["var"]["end_val"], test_config["var"]["step"])):
        # calculate rank
        rank = i
        total_rank = rank * len(clients)
        # prepare hosts.txt
        with open("/tmp/hosts.txt", 'w') as f:
            for client in clients:
                f.write(f"{client} slots={rank}\n")
        # prepare output_folder
        sub_folder = f"client_{len(clients)}_rank"
        sub_folder = sub_folder + f"_{i}" 
        output_folder = os.path.join(output_base, sub_folder)
        os.makedirs(output_folder, exist_ok=True)
        subprocess.run(f"chmod +777 {output_folder}", shell=True)
        # prepare run_config
        run_config = dlio_config
        run_config["output"]["folder"] = output_folder
        with open("/tmp/run.yaml", 'w', encoding='utf-8') as f:
            yaml.safe_dump(
                run_config, f,
                allow_unicode=True,
                sort_keys=False,
                indent=2
            )
        print("[info] copy yaml")
        copy_yaml(clients)
        # drop cache
        drop_caches(hosts)
        drop_caches(clients)
        # start sar
        print("[info] start sar")
        start_sar(hosts)
        time.sleep(60)
        # start benchmark
        print("[info] start benchmark")
        mpi_cmd=f"mpirun -mca btl_tcp_if_include 10.118.0.0/24 --allow-run-as-root -np {total_rank} --hostfile /tmp/hosts.txt /tmp/dlio_perf --config /tmp/run.yaml"
        # mpi_cmd=f"sudo -u hadoop mpirun -mca btl_tcp_if_include 10.118.0.0/24 -np {total_rank} --hostfile /tmp/hosts.txt /tmp/dlio_perf --config /tmp/run.yaml"
        subprocess.run(mpi_cmd, shell=True)
        # stop sar
        print("[info] stop sar")
        time.sleep(180)
        stop_sar(hosts, output_folder)
        rm_ck(dlio_config["checkpoint"]["checkpoint_folder"])
        time.sleep(60)
    rm_cmd = "rm /tmp/run.yaml"
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