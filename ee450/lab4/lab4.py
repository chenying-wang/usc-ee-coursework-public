#!/usr/bin/python3

import sys

START_FLAG = 'SF'
END_FLAG = 'EF'

def main(fname):
    with open(fname) as f:
        lines = f.readlines()
    lines = [line.strip() for line in lines]

    # 0 Read input file
    sources_name = []
    sources_packets = []
    sources_priority = []
    for line in lines:
        if not line.startswith('Source'):
            sources = line.split(' ')
            for source in sources:
                sources_priority.append(sources_name.index(source))
            del sources
            continue
        [source, packets] = line.split(':')
        source = source.strip()
        packets = packets.strip()
        sources_name.append(source[6:])
        source_packets = {}
        for packet in packets.split(','):
            packet = packet.strip()
            [start, end, name] = packet.split(' ')
            start = int(start)
            length = int(end) - start
            source_packets[start] = {'name': name, 'len': length}
        sources_packets.append(source_packets)
    del lines

    # 1 Calculate transmission rate
    n_packets, begin, end, max_length = 0, 0, 0, 0
    for source_packets in sources_packets:
        for [start, data] in source_packets.items():
            length = data['len']
            n_packets +=length
            begin = min(start, begin)
            end = max(start + length, end)
            max_length = max(length, max_length)
    trans_rate = -float(n_packets // (begin - end))
    frame_time = max_length / trans_rate

    # 2 TDM
    packets_sent, from_time = 0, float(begin)
    print(START_FLAG)
    while packets_sent < n_packets:
        min_start = end
        for source_packets in sources_packets:
            if not source_packets:
                continue
            min_start = min(min(source_packets.keys()), min_start)
        for idx in sources_priority:
            if min_start in sources_packets[idx]:
                source_idx = idx
                break
        if from_time > 0 and not from_time % frame_time:
            print(END_FLAG)
            print(START_FLAG)
        name = sources_packets[source_idx][min_start]['name']
        length = sources_packets[source_idx][min_start]['len']
        to_time = from_time + length / trans_rate
        print('%d, %.2f %.2f %s' % (source_idx, from_time, to_time, name))
        sources_packets[source_idx].pop(min_start)
        from_time = to_time
        packets_sent += length
    print(END_FLAG)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print('Please specify the input file')
        exit(1)
    main(sys.argv[1])
