#pragma once

void capSenseThread();


extern Queue<int32_t, 10> swipeQueue;
extern MemoryPool<int32_t, 10> swipePool;
