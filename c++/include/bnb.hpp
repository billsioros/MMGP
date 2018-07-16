
#pragma once

#include <functional>
#include <vector>

namespace BNB
{
    template <typename T>
    using Candidate = std::vector<T>;

    template <typename T>
    void BNB(const Candidate<T>&, Candidate<T>&,
        const std::function<double(const Candidate<T>&, const Candidate<T>&)>);
}

#include "bnb.ipp"
