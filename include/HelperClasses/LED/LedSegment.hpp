#pragma once

#include <FastLED.h>
#include <vector>
#include <cstdint>

class LedSegment {
public:
    // Default — empty/null segment
    LedSegment()
        : _length(0), _indexed(false), _contiguous(nullptr), _master(nullptr)
    {}

    // Contiguous segment — offset + length into master buffer
    LedSegment(CRGB* master, size_t offset, size_t length)
        : _length(length), _indexed(false)
    {
        _contiguous = master + offset;
    }

    // Indexed segment — arbitrary indices into master buffer
    LedSegment(CRGB* master, std::vector<size_t> indices)
        : _length(indices.size()), _indexed(true), _indices(std::move(indices))
    {
        _master = master;
    }

    size_t length() const { return _length; }

    CRGB& operator[](size_t i)
    {
        if (_indexed) {
            return _master[_indices[i]];
        }
        return _contiguous[i];
    }

    const CRGB& operator[](size_t i) const
    {
        if (_indexed) {
            return _master[_indices[i]];
        }
        return _contiguous[i];
    }

    void fill(CRGB color)
    {
        for (size_t i = 0; i < _length; i++) {
            (*this)[i] = color;
        }
    }

    void clear()
    {
        fill(CRGB::Black);
    }

    // Iterator support so patterns can use range-for
    struct Iterator {
        LedSegment* seg;
        size_t idx;

        CRGB& operator*() { return (*seg)[idx]; }
        Iterator& operator++() { ++idx; return *this; }
        bool operator!=(const Iterator& other) const { return idx != other.idx; }
    };

    Iterator begin() { return { this, 0 }; }
    Iterator end()   { return { this, _length }; }

private:
    size_t _length;
    bool _indexed;

    // Contiguous mode
    CRGB* _contiguous = nullptr;

    // Indexed mode
    CRGB* _master = nullptr;
    std::vector<size_t> _indices;
};