import re
import sys

map_labels = {}

def parse_map(name: str):
    with open(name, 'r') as f:
        for line in f:
            m = re.match(r'\s+([0-9A-F]+)\s+([\w]+)', line)
            if m:
                map_labels[m.group(2)] = int(m.group(1), 16)
                # print(m.groups())

lst_labels = {}

def parse_lst(name: str):
    # first pass: collect all labels with their area and lst offset
    raw = []  # list of (lst_offset, label_name, area_name)
    current_area = None
    with open(name, 'r') as f:
        for line in f:
            # track which area we're in
            m = re.match(r'\s*\d+\s*\.area\s+(\w+)', line)
            if m:
                current_area = m.group(1)
                continue
            # collect labels
            m = re.match(r'\s+([0-9A-F]+)\s+\d+\s+([^:]+):+',line)
            if m:
                if m.group(2).endswith('$'):
                    continue
                raw.append((int(m.group(1), 16), m.group(2), current_area))

    # print(f"DEBUG: raw labels in _CODE: {[l for o,l,a in raw if a=='_CODE']}")
    # print(f"DEBUG: map_labels keys sample: {list(map_labels.keys())[:10]}")
    # second pass: find one anchor per area (a label present in both lst and map)
    anchors = {}  # area_name -> (lst_offset, abs_address)
    for lst_offset, label, area in raw:
        if area not in anchors and label in map_labels:
            anchors[area] = (lst_offset, map_labels[label])
            # print(f"anchor for {area}: {label} lst={lst_offset:04x} abs={map_labels[label]:04x}")

    # third pass: resolve all labels using their area's anchor
    for lst_offset, label, area in raw:
        if label in map_labels:
            # already have it from map, skip
            continue
        if area not in anchors:
            # print(f"warning: no anchor for area {area}, skipping {label}")
            continue
        anchor_lst, anchor_abs = anchors[area]
        abs_addr = anchor_abs + (lst_offset - anchor_lst)
        lst_labels[label] = abs_addr
        # print(f"lst label {label} ({area}) = {abs_addr:04x}")

parse_map("out.map")
for file in sys.argv[1:]:
    parse_lst(file)

for l in map_labels:
    if l.startswith("s__") or l.startswith("l__"):
        continue
    print(f"{map_labels[l]:04x} {l}")
for l in lst_labels:
    print(f"{lst_labels[l]:04x} {l}")
