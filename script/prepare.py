#!/usr/bin/python3

import os

root = os.environ.get("ROOT_DIR", ".")
os.chdir(root)


def compute_thread_list(n):
    """Build a stable list of thread counts for scaling experiments.

    Avoid oversubscription (n+1) and too dense noisy points near n/2 and n.
    """
    if n is None or n <= 0:
        return [1]
    points = {1, 2, 3, 4, 
              n // 2, n // 2 + 2, 
              (2 * n) // 3, (2 * n) // 3 + 2,
              n - 2, n, n + 2, n + 4}
    return sorted(set(points))

def update_config(config_path, thread_list):
    """ Edit conf/common.conf """
    with open(config_path, "w") as f:
        f.write(
            "\n# \"script/prepare.py\" wrote this\n\n"
            "runs=7\n\n"
            "thread_list=(\n")
        for thread in thread_list:
            f.write(f"    {thread}\n")
        f.write(")\n")


if __name__ == '__main__':
    nproc = os.cpu_count()
    thread_list = compute_thread_list(nproc)
    config_file = 'conf/common.conf'
    update_config(config_file, thread_list)
    print(f"nproc = {nproc}")
    print(f"thread_list = ({' '.join(str(x) for x in thread_list)})")
