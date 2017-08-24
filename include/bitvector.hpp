#ifndef BITVECTOR_H_
#define BITVECTOR_H_

#include <assert.h>

#include <vector>

#include "config.hpp"

namespace surf {

class BitVector {
public:
    BitVector() : num_bits_(0), bits_(NULL) {};

    BitVector(const std::vector<std::vector<word_t> > &bitvector_per_level, const std::vector<position_t> &num_bits_per_level) {
	num_bits_ = computeTotalNumBits(num_bits_per_level);
	bits_ = new word_t[num_bits_/kWordSize + 1];
	concatenateBitVectors(bitvector_per_level, num_bits_per_level);
    }

    ~BitVector() {
	delete[] bits_;
    }

    position_t size() const {
        return (sizeof(BitVector) + (num_bits_ >> 3));
    }

    position_t numWords() const {
	return (num_bits_ >> 6);
    }

    //TODO: maybe not necessary
    void setBit(const position_t pos) {
        assert(pos <= num_bits_);
        position_t word_id = pos / kWordSize;
        position_t offset = pos & (kWordSize - 1);
        bits_[word_id] |= (kMsbMask >> offset);
    }

    bool readBit (const position_t pos) const {
        assert(pos <= num_bits_);
        position_t word_id = pos / kWordSize;
        position_t offset = pos & (kWordSize - 1);
        return bits_[word_id] & (kMsbMask >> offset);
    }

    bool operator[] (const position_t pos) const {
	return readBit(pos);
    }

    position_t distanceToNextOne(const position_t pos) const {
	assert(pos <= num_bits_);
	position_t distance = 1;

	position_t word_id = (pos + 1) / kWordSize;
	if (word_id >= numWords()) return distance;
	position_t offset = (pos + 1) % kWordSize;

	//first word left-over bits
	word_t test_bits = bits_[word_id] << offset;
	if (test_bits > 0)
	    return (distance + __builtin_clzll(test_bits));
	else
	    distance += (kWordSize - offset);

	while (true) {
	    word_id++;
	    if (word_id >= numWords()) return distance;
	    test_bits = bits_[word_id];
	    if (test_bits > 0)
		return (distance + __builtin_clzll(test_bits));
	    distance += kWordSize;
	}
    }

private:
    position_t computeTotalNumBits(std::vector<position_t> num_bits_per_level) {
	for (level_t level = 0; level < num_bits_per_level.size(); level++) {
	    num_bits_ += num_bits_per_level[level];
	}	
    }

    void concatenateBitVectors(std::vector<std::vector<word_t> > bitvector_per_level, std::vector<position_t> num_bits_per_level) {
        position_t bit_shift = 0;
        position_t word_id = 0;
        for (level_t level = 0; level < bitvector_per_level.size(); level++) {
            position_t num_complete_words = num_bits_per_level[level] / kWordSize;
            for (position_t word = 0; word < num_complete_words; word++) {
                bits_[word_id] |= (bitvector_per_level[level][word] >> bit_shift);
                word_id++;
                bits_[word_id] |= (bitvector_per_level[level][word] << (kWordSize - bit_shift));
            }
            word_t last_word = bitvector_per_level[level][num_complete_words];
            word_t bits_remain = num_bits_per_level[level] - num_complete_words * kWordSize;
            bits_[word_id] |= (last_word >> bit_shift);
            if (bit_shift + bits_remain < kWordSize) {
                bit_shift += bits_remain;
            }
            else {
                word_id++;
                bits_[word_id] |= (last_word << (kWordSize - bit_shift));
                bit_shift = bit_shift + bits_remain - kWordSize;
            }
        }
    }

protected:
    position_t num_bits_;
    word_t* bits_;
};

} // namespace surf

#endif // BITVECTOR_H_
