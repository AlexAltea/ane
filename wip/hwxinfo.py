#!/usr/bin/env -S uv run --script
# /// script
# dependencies = [
#   "macholib==1.16.3",
# ]
# ///

import sys
from macholib.MachO import MachO

def HWX(path):
    data = open(path, "rb").read()
    data = b"\xcf\xfa\xed\xfe" + data[4:]
    from tempfile import NamedTemporaryFile
    with NamedTemporaryFile(delete=False) as f:
        f.write(data)
        f.close()
    return MachO(f.name, allow_unknown_load_commands=True)

def get_section(hwx, segmentName, sectionName):
    for header in hwx.headers:
        for idx, (lc, cmd, data) in enumerate(header.commands):
            if cmd.segname.rstrip(b"\x00").decode("utf-8", "ignore") != segmentName:
                continue
            for section in data:
                if section.sectname.rstrip(b"\x00").decode("utf-8", "ignore") == sectionName and \
                   section.segname.rstrip(b"\x00").decode("utf-8", "ignore") == segmentName:
                    return section
    return None

def get_segments(hwx, segmentName):
    for header in hwx.headers:
        for idx, (lc, cmd, data) in enumerate(header.commands):
            if cmd.segname.rstrip(b"\x00").decode("utf-8", "ignore") == segmentName:
                yield lc, cmd, data
  
def list_load_commands(filename):
    hwx = HWX(filename)

    for header in hwx.headers:
        print(f"Header for {filename}")
        for idx, (lc, cmd, data) in enumerate(header.commands):
            name = lc.get_cmd_name()
            print(f"  [{idx}] Load command: {lc.cmd} ({lc.get_cmd_name()})")
            print(f"      Size: {lc.cmdsize}")

def main():
    if len(sys.argv) < 2:
        print(f"Usage: ./hwxinfo.py <path/to/model.hwx>")
        sys.exit(1)

    hwx = HWX(sys.argv[1])
    tsk_section = get_section(hwx, '__TEXT', '__text')
    tsk_addr = tsk_section.addr
    tsk_size = tsk_section.size

    krn_section = get_section(hwx, '__TEXT', '__const')
    krn_addr = krn_section.addr
    krn_size = krn_section.size

    print(f'{tsk_addr=:#x}')
    print(f'{tsk_size=:#x}')
    print(f'{krn_addr=:#x}')
    print(f'{krn_size=:#x}')

    itm_count = 0
    src_count = 0
    dst_count = 0
    for idx, (lc, cmd, data) in enumerate(hwx.headers[0].commands):
        if lc.cmd != 25 or cmd.segname.rstrip(b"\x00").decode("utf-8", "ignore") != '__FVMLIB':
            continue
        assert(cmd.initprot == cmd.maxprot)
        if cmd.initprot == 0x3: # RW
            print(f'itm{itm_count}: size={cmd.vmsize}')
            itm_count += 1
        elif cmd.initprot == 0x1: # R-only
            print(f'src{src_count}: size={cmd.vmsize}')
            src_count += 1
        elif cmd.initprot == 0x2: # W-only
            print(f'dst{dst_count}: size={cmd.vmsize}')
            dst_count += 1
            
        

    #list_load_commands(sys.argv[1])

if __name__ == "__main__":
    main()

