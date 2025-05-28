void PriorityQueue::Generate(PT pt)
{
    // 计算PT的概率，这里主要是给PT的概率进行初始化
    CalProb(pt);
    int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    // 对于只有一个segment的PT，直接遍历生成其中的所有value即可
    if (pt.content.size() == 1)
    {
        // 指向最后一个segment的指针，这个指针实际指向模型中的统计数据
        segment *a;
        // 在模型中定位到这个segment
        if (pt.content[0].type == 1)
        {
            a = &m.letters[m.FindLetter(pt.content[0])];
        }
        if (pt.content[0].type == 2)
        {
            a = &m.digits[m.FindDigit(pt.content[0])];
        }
        if (pt.content[0].type == 3)
        {
            a = &m.symbols[m.FindSymbol(pt.content[0])];
        }

        // Multi-thread TODO：
        // 这个for循环就是你需要进行并行化的主要部分了，特别是在多线程&GPU编程任务中
        // 可以看到，这个循环本质上就是把模型中一个segment的所有value，赋值到PT中，形成一系列新的猜测
        // 这个过程是可以高度并行化的



        //原代码

        // for (int i = 0; i < pt.max_indices[0]; i += 1)
        // {
        //     string guess = a->ordered_values[i];
        //     // cout << guess << endl;
        //     guesses.emplace_back(guess);
        //     total_guesses += 1;
        // }        


        //openmp

        // int max_idx = pt.max_indices[0];
        // int val_len = a->ordered_values[0].size();
        // int old_size = guesses.size();
        // guesses.resize(old_size + max_idx);
        // #pragma omp parallel for schedule(static)
        // for (int i = 0; i < max_idx; ++i)
        // {
        //     guesses[old_size + i] = std::string(a->ordered_values[i].data(), val_len);
        // }
        // total_guesses += max_idx;



        //buffer/openmp2

        int max_idx = pt.max_indices[0];
        int val_len = a->ordered_values[0].size();
        int old_size = guesses.size();
        guesses.resize(old_size + max_idx);
        std::vector<std::vector<std::string>> buffers(omp_get_max_threads());
        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            std::vector<std::string>& local = buffers[tid];
            #pragma omp for schedule(static)
            for (int i = 0; i < max_idx; ++i)
                local.emplace_back(a->ordered_values[i].data(), val_len);
        }
        for (auto& buf : buffers)
            guesses.insert(guesses.end(), buf.begin(), buf.end());
        total_guesses += max_idx;






    }
    else
    {
        string guess;
        int seg_idx = 0;
        // 这个for循环的作用：给当前PT的所有segment赋予实际的值（最后一个segment除外）
        // segment值根据curr_indices中对应的值加以确定
        // 这个for循环你看不懂也没太大问题，并行算法不涉及这里的加速
        for (int idx : pt.curr_indices)
        {
            if (pt.content[seg_idx].type == 1)
            {
                guess += m.letters[m.FindLetter(pt.content[seg_idx])].ordered_values[idx];
            }
            if (pt.content[seg_idx].type == 2)
            {
                guess += m.digits[m.FindDigit(pt.content[seg_idx])].ordered_values[idx];
            }
            if (pt.content[seg_idx].type == 3)
            {
                guess += m.symbols[m.FindSymbol(pt.content[seg_idx])].ordered_values[idx];
            }
            seg_idx += 1;
            if (seg_idx == pt.content.size() - 1)
            {
                break;
            }
        }

        // 指向最后一个segment的指针，这个指针实际指向模型中的统计数据
        segment *a;
        if (pt.content[pt.content.size() - 1].type == 1)
        {
            a = &m.letters[m.FindLetter(pt.content[pt.content.size() - 1])];
        }
        if (pt.content[pt.content.size() - 1].type == 2)
        {
            a = &m.digits[m.FindDigit(pt.content[pt.content.size() - 1])];
        }
        if (pt.content[pt.content.size() - 1].type == 3)
        {
            a = &m.symbols[m.FindSymbol(pt.content[pt.content.size() - 1])];
        }

        // Multi-thread TODO：
        // 这个for循环就是你需要进行并行化的主要部分了，特别是在多线程&GPU编程任务中
        // 可以看到，这个循环本质上就是把模型中一个segment的所有value，赋值到PT中，形成一系列新的猜测
        // 这个过程是可以高度并行化的



        // 原代码

        // for (int i = 0; i < pt.max_indices[pt.content.size() - 1]; i += 1)
        // {
        //     string temp = guess + a->ordered_values[i];
        //     // cout << temp << endl;
        //     guesses.emplace_back(temp);
        //     total_guesses += 1;
        // }



        //openmp

        // int max_idx = pt.max_indices.back();
        // int val_len = a->ordered_values[0].size();
        // int prefix_len = guess.size();
        // const char* prefix_ptr = guess.data();
        // int old_size = guesses.size();
        // guesses.resize(old_size + max_idx);
        // #pragma omp parallel for schedule(static)
        // for (int i = 0; i < max_idx; ++i)
        // {
        //     std::string& dest = guesses[old_size + i];
        //     dest.resize(prefix_len + val_len);
        //     memcpy(&dest[0], prefix_ptr, prefix_len);
        //     memcpy(&dest[0] + prefix_len, a->ordered_values[i].data(), val_len);
        // }
        // total_guesses += max_idx;









        //buffer/openmp2

        int max_idx = pt.max_indices.back();
        int val_len = a->ordered_values[0].size();
        int prefix_len = guess.size();
        const char* prefix_ptr = guess.data();
        int old_size = guesses.size();
        guesses.resize(old_size + max_idx);
        std::vector<std::vector<std::string>> buffers(omp_get_max_threads());
        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            std::vector<std::string>& local = buffers[tid];
            #pragma omp for schedule(static)
            for (int i = 0; i < max_idx; ++i) {
                std::string s;
                s.resize(prefix_len + val_len);
                memcpy(&s[0], prefix_ptr, prefix_len);
                memcpy(&s[0] + prefix_len, a->ordered_values[i].data(), val_len);
                local.emplace_back(std::move(s));
            }
        }
        for (auto& buf : buffers)
            guesses.insert(guesses.end(), buf.begin(), buf.end());
        total_guesses += max_idx;




    }
}
