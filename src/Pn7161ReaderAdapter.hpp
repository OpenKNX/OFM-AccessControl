#pragma once
#include "IsoDepReaderAdapter.hpp"
#include <Pn7161Reader.hpp>

class Pn7161ReaderAdapter final : public IsoDepReaderAdapter<Pn7161Reader,true> {
public:
    Pn7161ReaderAdapter(const Pn7161Reader::Config& config,
                       const std::array<uint8_t,18>& frame)
        : IsoDepReaderAdapter(frame,config) {}
};
