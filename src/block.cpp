#include <algorithm>
#include <vector>
#include "block.hpp"
#include "koopa.h"

static const void **vector_data(std::vector<void *> &vec)
{
    auto buffer = new const void *[vec.size()];
    std::copy(vec.begin(), vec.end(), buffer);

    return buffer;
}

void BlockInst::set_block(std::vector<void *> *_block)
{
    block = _block;

    return;
}

void BlockInst::new_block(koopa_raw_basic_block_data_t *basic)
{
    end_block();
    basic->insts.buffer = nullptr;
    block->push_back(basic);

    return;
}

void BlockInst::end_block(void)
{
    if(!block->empty())
    {
        for(auto &i : current)
        {
            koopa_raw_value_t t = (koopa_raw_value_t)i;
            if(t->kind.tag == KOOPA_RVT_BRANCH || t->kind.tag == KOOPA_RVT_RETURN || t->kind.tag == KOOPA_RVT_JUMP)
            {
                current.resize(&i - current.data() + 1);
                break;
            }
        }

        koopa_raw_basic_block_data_t *last = (koopa_raw_basic_block_data_t *)(*block->rbegin());
        if(!last->insts.buffer)
            last->insts = {vector_data(current), (unsigned)current.size(), KOOPA_RSIK_VALUE};
    }
    current.clear();

    return;
}

void BlockInst::add_inst(void *inst)
{
    current.push_back(inst);

    return;
}
