struct ThreadArg {
    int tid;
    int num_threads;
    std::function<void(int, int)> func;
};

void* wrapper(void* arg) {
    ThreadArg* t = static_cast<ThreadArg*>(arg);
    t->func(t->tid, t->num_threads);
    return nullptr;
}

void PriorityQueue::Generate(PT pt)
{
    CalProb(pt);
    int num_threads = 4;

    if (pt.content.size() == 1)
    {
        segment *a;
        if (pt.content[0].type == 1)
            a = &m.letters[m.FindLetter(pt.content[0])];
        else if (pt.content[0].type == 2)
            a = &m.digits[m.FindDigit(pt.content[0])];
        else
            a = &m.symbols[m.FindSymbol(pt.content[0])];

        int max_idx = pt.max_indices[0];
        int val_len = a->ordered_values[0].size();
        int old_size = guesses.size();
        guesses.resize(old_size + max_idx, std::string(val_len, '\0')); // allocate space

        std::vector<ThreadArg> args(num_threads);
        std::vector<pthread_t> threads(num_threads);

        for (int i = 0; i < num_threads; ++i) {
            args[i] = ThreadArg{
                .tid = i,
                .num_threads = num_threads,
                .func = [=](int tid, int thread_num) {
                    int chunk_size = max_idx / thread_num;
                    int remainder = max_idx % thread_num;
                    int begin = tid * chunk_size + std::min(tid, remainder);
                    int end = begin + chunk_size + (tid < remainder ? 1 : 0);

                    for (int j = begin; j < end; ++j) {
                        char* dest = &guesses[old_size + j][0];
                        std::memcpy(dest, a->ordered_values[j].data(), val_len);
                    }
                }
            };
            pthread_create(&threads[i], nullptr, wrapper, &args[i]);
        }

        for (int i = 0; i < num_threads; ++i)
            pthread_join(threads[i], nullptr);

        total_guesses += max_idx;
    }
    else
    {
        std::string guess;
        int seg_idx = 0;
        for (int idx : pt.curr_indices)
        {
            if (pt.content[seg_idx].type == 1)
                guess += m.letters[m.FindLetter(pt.content[seg_idx])].ordered_values[idx];
            else if (pt.content[seg_idx].type == 2)
                guess += m.digits[m.FindDigit(pt.content[seg_idx])].ordered_values[idx];
            else
                guess += m.symbols[m.FindSymbol(pt.content[seg_idx])].ordered_values[idx];

            seg_idx += 1;
            if (seg_idx == pt.content.size() - 1)
                break;
        }

        segment *a;
        if (pt.content.back().type == 1)
            a = &m.letters[m.FindLetter(pt.content.back())];
        else if (pt.content.back().type == 2)
            a = &m.digits[m.FindDigit(pt.content.back())];
        else
            a = &m.symbols[m.FindSymbol(pt.content.back())];

        int max_idx = pt.max_indices.back();
        int val_len = a->ordered_values[0].size();
        int prefix_len = guess.size();
        const char* prefix_ptr = guess.data();
        int old_size = guesses.size();
        guesses.resize(old_size + max_idx, std::string(prefix_len + val_len, '\0')); // allocate space

        std::vector<ThreadArg> args(num_threads);
        std::vector<pthread_t> threads(num_threads);

        for (int i = 0; i < num_threads; ++i) {
            args[i] = ThreadArg{
                .tid = i,
                .num_threads = num_threads,
                .func = [=](int tid, int thread_num) {
                    int chunk_size = max_idx / thread_num;
                    int remainder = max_idx % thread_num;
                    int begin = tid * chunk_size + std::min(tid, remainder);
                    int end = begin + chunk_size + (tid < remainder ? 1 : 0);

                    for (int j = begin; j < end; ++j) {
                        char* dest = &guesses[old_size + j][0];
                        std::memcpy(dest, prefix_ptr, prefix_len);
                        std::memcpy(dest + prefix_len, a->ordered_values[j].data(), val_len);
                    }
                }
            };
            pthread_create(&threads[i], nullptr, wrapper, &args[i]);
        }

        for (int i = 0; i < num_threads; ++i)
            pthread_join(threads[i], nullptr);

        total_guesses += max_idx;
    }
}
