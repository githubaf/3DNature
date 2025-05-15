import os
import hashlib
import re
import argparse

def calculate_md5(file_path):
    """Calculate MD5 checksum of a file."""
    hash_md5 = hashlib.md5()
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()

def find_files(base_dir_pattern, file_name_pattern):
    """Find all files in directories matching the base_dir_pattern."""
    matched_files = {}
    matched_dirs = set()
    
    for root, dirs, files in os.walk('.'):
        if re.match(base_dir_pattern, os.path.basename(root)):
            matched_dirs.add(root)
            for file in files:
                if re.match(file_name_pattern, file):
                   file_path = os.path.join(root, file)
                   md5_checksum = calculate_md5(file_path)
                   if md5_checksum not in matched_files:
                       matched_files[md5_checksum] = []
                   matched_files[md5_checksum].append(file_path)
    return matched_files, len(matched_dirs)

def main():
    parser = argparse.ArgumentParser(description="Find identical files in directories matching a pattern.")
    parser.add_argument("base_dir_pattern", type=str, help="Pattern to match base directories.")
    parser.add_argument("file_name_pattern", type=str, help="Pattern to match file names.")
    args = parser.parse_args()

    matched_files, total_dirs = find_files(args.base_dir_pattern, args.file_name_pattern)

    for md5_checksum, files in matched_files.items():
        files.sort()
        if len(files) == total_dirs:
            print(f"\033[92mMD5: {md5_checksum}\033[0m")
            for file in files:
                print(f"\033[92m{file}\033[0m")
        else:
            print(f"MD5: {md5_checksum}")
            for file in files:
                print(file)

if __name__ == "__main__":
    main()


