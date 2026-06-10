module utils.consts;
@nogc nothrow:

enum size_t BLOCK_SIZE = 64;

enum size_t REFS_MASK = ~(cast(size_t) 0) >> 1;
enum size_t SHOULD_BE_ON_LIST = ~REFS_MASK;
