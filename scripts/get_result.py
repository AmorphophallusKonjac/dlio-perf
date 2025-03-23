import yaml
import argparse
import os
import re
import subprocess

def find_x_subfolder(target_dir, x):
    # 验证输入x是否为整数
    if not isinstance(x, int):
        raise ValueError("x必须是整数")
    
    # 构建正则表达式模式：严格匹配以_x结尾的文件夹名
    pattern = re.compile(rf'_{x}$')  # 动态生成正则表达式
    
    matched_folders = []
    # 遍历目标目录的一级子项（不递归）
    for entry in os.listdir(target_dir):
        full_path = os.path.join(target_dir, entry)
        if os.path.isdir(full_path) and pattern.search(entry):
            matched_folders.append(full_path)
    
    return matched_folders


def main(args):
    folder = args.dir
    file_path = os.path.join(folder, "var.yaml")
    with open(file_path, 'r', encoding='utf-8') as f:
        test_config = yaml.safe_load(f)
    for i in range(test_config["var"]["start_val"], test_config["var"]["end_val"], test_config["var"]["step"]):
        sub_dir= find_x_subfolder(folder, i)[0]
        result_yaml_path = os.path.join(sub_dir, "all_rank_result.yaml")
        with open(result_yaml_path, 'r', encoding='utf-8') as f:
            result = yaml.safe_load(f)
        
    

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--dir', required=True, help='result dir')
    args = parser.parse_args()
    main(args)