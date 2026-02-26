#!/usr/bin/env python3
import sys
import re

def parse_map(map_file):
    areas = {}
    labels = {}
    with open(map_file) as f:
        for line in f:
            m = re.match(r'\s+([0-9A-F]+)\s+s_(_\w+)', line)
            if m:
                areas[m.group(2)] = int(m.group(1), 16)
                continue
            m = re.match(r'\s+([\dA-F]+)\s+(\w+).*', line)
            if m and not m.group(2).startswith("l__"):
                labels[m.group(2)] = int(m.group(1), 16)
    return areas, labels

def parse_lst(lst_file, areas, existing_labels, current_area=None):
    labels = {}
    with open(lst_file) as f:
        for line in f:
            line = line.removesuffix("\n")
            m = re.match(r'\s*\d+\s*\.area\s+(\w+)', line)
            if m:
                current_area = m.group(1)
                continue
            m = re.match(r'\s+([\dA-F]+)\s+(\d+)\s+(\w+):', line)
            if m and current_area in areas:
                label = m.group(3)
                if label not in existing_labels:
                    offset = int(m.group(1), 16)
                    labels[label] = areas[current_area] + offset
    return labels

if len(sys.argv) < 2:
    print("usage: parse_labels.py out.map [file1.lst ...]")
    sys.exit(1)

areas, labels = parse_map(sys.argv[1])
for lst in sys.argv[2:]:
    labels.update(parse_lst(lst, areas, labels))

for label, addr in sorted(labels.items(), key=lambda x: x[1]):
    print(f"{addr:04x} {label}")
