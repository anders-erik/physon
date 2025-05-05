
# Fetch and build reference projects


import os
import subprocess

# Ensure the ./references folders exist
references_dir = os.path.join(os.getcwd(), "ref")
include_dir = os.path.join(references_dir, "include")
lib_dir = os.path.join(references_dir, "lib")

os.makedirs(references_dir, exist_ok=True)
os.makedirs(include_dir, exist_ok=True)
os.makedirs(lib_dir, exist_ok=True)




# function to fetch C-Simple-JSON-Parser
def fetch_c_simple_json_parser():

    c_simple_dir=os.path.join(references_dir, "C-Simple-JSON-Parser")
    c_simple_include_dir = os.path.join(include_dir, "c_simple")

    c_simple_header_src = os.path.join(c_simple_dir, "json.h")
    c_simple_header_dist = os.path.join(c_simple_include_dir, "json.h")

    os.makedirs(c_simple_include_dir, exist_ok=True)

    # Delete repo if it exists
    if os.path.exists(c_simple_dir):
        subprocess.run(["rm", "-rf", c_simple_dir], check=True)

    # Clone repo
    c_simple_url = "https://github.com/whyisitworking/C-Simple-JSON-Parser.git"
    subprocess.run(["git", "clone", c_simple_url, c_simple_dir], check=True)

    # copy header to references/include
    subprocess.run(["cp", c_simple_header_src, c_simple_header_dist], check=True)


def build_c_simple_json_parser():
    # build c_simple_json_parser
    c_simple_dir = os.path.join(references_dir, "C-Simple-JSON-Parser")
    c_simple_c_file = os.path.join(c_simple_dir, "json.c")
    # c_simple_lib_file = os.path.join(lib_dir, "c_simple.so")
    c_simple_lib_file = os.path.join(lib_dir, "c_simple.o")


    # Delete lib file if it exists
    if os.path.exists(c_simple_lib_file):
        subprocess.run(["rm", c_simple_lib_file], check=True)

    # Build the library
    # subprocess.run(["gcc", "-shared", "-o", c_simple_lib_file, "-fPIC", c_simple_c_file], check=True)
    # build object file
    subprocess.run(["gcc", "-c", c_simple_c_file, "-o", c_simple_lib_file], check=True)

def fetch_nlohmann_json():
    nlohmann_dir = os.path.join(references_dir, "nlohmann_json")

    nlohmann_include_src_dir = os.path.join(nlohmann_dir, "include/nlohmann")
    nlohmann_include_dist_dir = os.path.join(include_dir, "nlohmann")

    # nlohmann_header_src = os.path.join(nlohmann_dir, "json.hpp")
    # nlohmann_header_dist = os.path.join(nlohmann_include_dir, "json.hpp")

    # os.makedirs(nlohmann_include_dir, exist_ok=True)

    # Delete repo if it exists
    if os.path.exists(nlohmann_dir):
        subprocess.run(["rm", "-rf", nlohmann_dir], check=True)

    # Clone repo
    nlohmann_url = "https://github.com/nlohmann/json.git"
    subprocess.run(["git", "clone", nlohmann_url, nlohmann_dir], check=True)

    # copy headers to references/include
    subprocess.run(["cp", "-r", nlohmann_include_src_dir, nlohmann_include_dist_dir], check=True)

fetch_nlohmann_json()

# fetch_c_simple_json_parser()
# build_c_simple_json_parser()