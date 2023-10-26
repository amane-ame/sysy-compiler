#pragma once

#include <vector>
#include "koopa.h"

class BlockInst
{
private:
    std::vector<void *> current, *block;

public:
    void set_block(std::vector<void *> *_block);
    void new_block(koopa_raw_basic_block_data_t *basic);
    void end_block(void);
    void add_inst(void *inst);
};
