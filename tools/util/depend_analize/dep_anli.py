import os

CURRENT_PATH = os.path.abspath(os.path.dirname(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(CURRENT_PATH, "../../.."))

def find_file(start:str, name:str):
    for relpath, _, files in os.walk(start):
        if name in files:
            full_path = os.path.join(start, relpath, name)
            return os.path.normpath(os.path.abspath(full_path))
    return None

"""
 * Description: 该函数用于分析指定文件所依赖的所有头文件，结果包含去重
"""
def analyze_file(file_abs_name:str, recursion:bool=False):
    if not file_abs_name.endswith((".h", ".c")):
        return list()
    if not os.path.exists(file_abs_name):
        return list()
    h_file_list = list()
    with open(file_abs_name, "r") as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith("#include"):
                h_file = line.split(" ")[1][1:-2]
                h_file_abs = find_file(PROJECT_ROOT, h_file)
                if h_file is None:
                    print("[Error]: {} not found!!!".format(h_file_abs))
                    continue
                h_file_list.append(h_file_abs)
                if recursion:
                    h_file_list += analyze_file(h_file_abs)
                h_file_list = list(set(h_file_list))
    return h_file_list

"""
 * Description: 该函数用于获取指定目录下的所有指定后缀的文件
"""
def find_all_files_with_specified_suffix(target_dir:str, target_suffix:str, expect_dirs:list):
    find_res = []
    target_suffix_dot = "." + target_suffix
    walk_generator = os.walk(target_dir)
    for root_path, _, files in walk_generator:
        if len(files) < 1:
            continue
        need_skip = False
        for exp_dir in expect_dirs:
            if root_path.startswith(exp_dir):
                need_skip = True
                break
        if need_skip:
            continue
        for file in files:
            __, suffix_name = os.path.splitext(file)
            if suffix_name == target_suffix_dot:
                find_res.append(os.path.join(root_path, file))
    return find_res

def analyze_head_file_relation():
    exp_dir = [os.path.join(PROJECT_ROOT, "tools"), 
               os.path.join(PROJECT_ROOT, "test"),
               os.path.join(PROJECT_ROOT, "test_suite"),
               ]
    files = find_all_files_with_specified_suffix(PROJECT_ROOT, "h", exp_dir)
    relation_map = dict()
    if len(files) == 0:
        return relation_map
    for f in files:
        relation_map[f] = analyze_file(f)
    return relation_map

if __name__ == "__main__":
    rmap = analyze_head_file_relation()
    for k, v in rmap.items():
        print(k)
        for vv in v:
            print("\t-->{}".format(vv))
        print("")