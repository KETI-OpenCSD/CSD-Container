// Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory)
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <thread>

#include "scan.h"

using namespace ROCKSDB_NAMESPACE;
using namespace std;

// ---------------현재 사용하는 인자(compression, cache X)-----------
// bool blocks_maybe_compressed = true;
// bool blocks_definitely_zstd_compressed = false;
// uint32_t read_amp_bytes_per_bit = 0;
// const bool immortal_table = false;
// std::string dev_name = "/dev/sda";
// -----------------------------------------------------------------

WorkQueue<Snippet> ScanQueue;
WorkQueue<ScanResult> FilterQueue;
WorkQueue<FilterResult> MergeQueue;

int main() {

    TableManager CSDTableManager = TableManager();
    CSDTableManager.InitCSDTableManager();

    // Return ReturnInterface = Return();

    thread InputInterface = thread(&Input::InputSnippet, Input());
    thread ScanLayer = thread(&Scan::Scanning, Scan(CSDTableManager));
    thread FilterLayer1 = thread(&Filter::Filtering, Filter());
    thread FilterLayer2 = thread(&Filter::Filtering, Filter());
    thread MergeLayer = thread(&MergeManager::Merging, MergeManager());

    // Input InputLayer = Input();
    // InputLayer.InputSnippet();

    InputInterface.join();
    ScanLayer.join();
    FilterLayer1.join();
    FilterLayer2.join();
    MergeLayer.join();
    
    return 0;
}
