#include "rng.hh"
#include "utils.hh"

namespace rng
{

static std::mt19937 create();

std::mt19937 mt {create()};

static std::mt19937
create()
{
    std::random_device rd {};
    std::seed_seq ss {
        (std::seed_seq::result_type)(timeNowS()), rd(), rd(), rd(), rd(), rd(), rd(), rd()
    };

    return std::mt19937(ss);
}

} /* namespace rng */
