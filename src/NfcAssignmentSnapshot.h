#pragma once
#include <cstdint>
#include <cstddef>
#include <algorithm>

namespace NfcAssignmentSnapshot {
inline void put32(uint8_t* out,uint32_t value) {
    out[0]=value>>24;out[1]=value>>16;out[2]=value>>8;out[3]=value;
}
// Read-only, generation-checked view of downloaded action assignments. No keys
// or arbitrary parameter addresses are exposed. Caller guarantees configured().
template<class Read> void page(const uint8_t* data,uint8_t* out,uint8_t& length,
    uint16_t count,uint16_t visible,bool homeKey,Read read) {
    const uint16_t cursor=(uint16_t(data[1])<<8)|data[2];
    const uint32_t wanted=(uint32_t(data[3])<<24)|(uint32_t(data[4])<<16)|(uint32_t(data[5])<<8)|data[6];
    uint32_t revision=2166136261u;
    auto hash=[&](uint16_t v){revision=(revision^uint8_t(v>>8))*16777619u;revision=(revision^uint8_t(v))*16777619u;};
    hash(count);hash(visible);hash(homeKey);
    for(uint16_t i=0;i<count;++i){hash(read(i,0));hash(read(i,1));}
    if(!revision)revision=1;
    out[0]=8;out[1]=1;length=2;
    if(cursor>count||((cursor||wanted)&&wanted!=revision))return;
    const uint8_t n=std::min<uint16_t>(32,count-cursor);
    out[0]=0;put32(out+2,revision);out[6]=(cursor+n)>>8;out[7]=cursor+n;out[8]=n;
    out[9]=count>>8;out[10]=count;out[11]=visible>>8;out[12]=visible;out[13]=homeKey;
    for(uint8_t i=0;i<n;++i){
        const uint16_t action=read(cursor+i,0),id=read(cursor+i,1);
        out[14+i*4]=action>>8;out[15+i*4]=action;out[16+i*4]=id>>8;out[17+i*4]=id;
    }
    length=14+n*4;
}
}
