#ifndef SBUF_PRIVATE_H
#define SBUF_PRIVATE_H

inline int sbuf_t::memcmp(const uint8_t *cbuf,size_t at,size_t len) const {
    if(left(at) < len) throw sbuf_t::range_exception_t();
    return ::memcmp(this->buf+at,cbuf,len);
}


/**
 * Unsigned get interfaces
 */
inline uint8_t sbuf_t::get8u(size_t i) const {
    if(i+1>bufsize) throw sbuf_t::range_exception_t();
    return this->buf[i];
}

inline uint16_t sbuf_t::get16u(size_t i) const {
    if(i+2>bufsize) throw sbuf_t::range_exception_t();
    return 0 
	| (uint16_t)(this->buf[i+0]<<0) 
	| (uint16_t)(this->buf[i+1]<<8);
}

inline uint32_t sbuf_t::get32u(size_t i) const {
    if(i+4>bufsize) throw sbuf_t::range_exception_t();
    return 0 
	| (uint32_t)(this->buf[i+0]<<0) 
	| (uint32_t)(this->buf[i+1]<<8) 
	| (uint32_t)(this->buf[i+2]<<16) 
	| (uint32_t)(this->buf[i+3]<<24);
}

inline uint64_t sbuf_t::get64u(size_t i) const {
    if(i+8>bufsize) throw sbuf_t::range_exception_t();
    return 0 
	| ((uint64_t)(this->buf[i+0])<<0) 
	| ((uint64_t)(this->buf[i+1])<<8) 
	| ((uint64_t)(this->buf[i+2])<<16) 
	| ((uint64_t)(this->buf[i+3])<<24) 
	| ((uint64_t)(this->buf[i+4])<<32) 
	| ((uint64_t)(this->buf[i+5])<<40) 
	| ((uint64_t)(this->buf[i+6])<<48) 
	| ((uint64_t)(this->buf[i+7])<<56);
}

inline uint8_t sbuf_t::get8uBE(size_t i) const {
    if(i+1>bufsize) throw sbuf_t::range_exception_t();
    return this->buf[i];
}

inline uint16_t sbuf_t::get16uBE(size_t i) const {
    if(i+2>bufsize) throw sbuf_t::range_exception_t();
    return 0 
	| (uint16_t)(this->buf[i+1]<<0) 
	| (uint16_t)(this->buf[i+0]<<8);
}

inline uint32_t sbuf_t::get32uBE(size_t i) const {
    if(i+4>bufsize) throw sbuf_t::range_exception_t();
    return 0 
	| (uint32_t)(this->buf[i+3]<<0) 
	| (uint32_t)(this->buf[i+2]<<8) 
	| (uint32_t)(this->buf[i+1]<<16) 
	| (uint32_t)(this->buf[i+0]<<24);
}

inline uint64_t sbuf_t::get64uBE(size_t i) const {
    if(i+8>bufsize) throw sbuf_t::range_exception_t();
    return 0 
	| ((uint64_t)(this->buf[i+7])<<0) 
	| ((uint64_t)(this->buf[i+6])<<8) 
	| ((uint64_t)(this->buf[i+5])<<16) 
	| ((uint64_t)(this->buf[i+4])<<24) 
	| ((uint64_t)(this->buf[i+3])<<32) 
	| ((uint64_t)(this->buf[i+2])<<40) 
	| ((uint64_t)(this->buf[i+1])<<48) 
	| ((uint64_t)(this->buf[i+0])<<56);
}

inline uint8_t sbuf_t::get8u(size_t i,sbuf_t::byte_order_t bo) const {
    return bo==BO_LITTLE_ENDIAN ? get8u(i) : get8uBE(i);
}

inline uint16_t sbuf_t::get16u(size_t i,sbuf_t::byte_order_t bo) const {
    return bo==BO_LITTLE_ENDIAN ? get16u(i) : get8uBE(i);
}

inline uint32_t sbuf_t::get32u(size_t i,sbuf_t::byte_order_t bo) const {
    return bo==BO_LITTLE_ENDIAN ? get32u(i) : get32uBE(i);
}

inline uint64_t sbuf_t::get64u(size_t i,sbuf_t::byte_order_t bo) const {
    return bo==BO_LITTLE_ENDIAN ? get64u(i) : get64uBE(i);
}

/**
 * Signed get interfaces
 */
inline int8_t sbuf_t::get8i(size_t i) const {
    if(i+1>bufsize) throw sbuf_t::range_exception_t();
    return this->buf[i];
}

inline int16_t sbuf_t::get16i(size_t i) const {
    if(i+2>bufsize) throw sbuf_t::range_exception_t();
    return 0 
	| (uint16_t)(this->buf[i+0]<<0) 
	| (uint16_t)(this->buf[i+1]<<8);
}

inline int32_t sbuf_t::get32i(size_t i) const {
    if(i+4>bufsize) throw sbuf_t::range_exception_t();
    return 0 
	| (uint32_t)(this->buf[i+0]<<0) 
	| (uint32_t)(this->buf[i+1]<<8) 
	| (uint32_t)(this->buf[i+2]<<16) 
	| (uint32_t)(this->buf[i+3]<<24);
}

inline int64_t sbuf_t::get64i(size_t i) const {
    if(i+8>bufsize) throw sbuf_t::range_exception_t();
    return 0 
	| ((uint64_t)(this->buf[i+0])<<0) 
	| ((uint64_t)(this->buf[i+1])<<8) 
	| ((uint64_t)(this->buf[i+2])<<16) 
	| ((uint64_t)(this->buf[i+3])<<24) 
	| ((uint64_t)(this->buf[i+4])<<32) 
	| ((uint64_t)(this->buf[i+5])<<40) 
	| ((uint64_t)(this->buf[i+6])<<48) 
	| ((uint64_t)(this->buf[i+7])<<56);
}

inline int8_t sbuf_t::get8iBE(size_t i) const {
    if(i+1>bufsize) throw sbuf_t::range_exception_t();
    return this->buf[i];
}

inline int16_t sbuf_t::get16iBE(size_t i) const {
    if(i+2>bufsize) throw sbuf_t::range_exception_t();
    return 0 
	| (uint16_t)(this->buf[i+1]<<0) 
	| (uint16_t)(this->buf[i+0]<<8);
}

inline int32_t sbuf_t::get32iBE(size_t i) const {
    if(i+4>bufsize) throw sbuf_t::range_exception_t();
    return 0 
	| (uint32_t)(this->buf[i+3]<<0) 
	| (uint32_t)(this->buf[i+2]<<8) 
	| (uint32_t)(this->buf[i+1]<<16) 
	| (uint32_t)(this->buf[i+0]<<24);
}

inline int64_t sbuf_t::get64iBE(size_t i) const {
    if(i+8>bufsize) throw sbuf_t::range_exception_t();
    return 0 
	| ((uint64_t)(this->buf[i+7])<<0) 
	| ((uint64_t)(this->buf[i+6])<<8) 
	| ((uint64_t)(this->buf[i+5])<<16) 
	| ((uint64_t)(this->buf[i+4])<<24) 
	| ((uint64_t)(this->buf[i+3])<<32) 
	| ((uint64_t)(this->buf[i+2])<<40) 
	| ((uint64_t)(this->buf[i+1])<<48) 
	| ((uint64_t)(this->buf[i+0])<<56);
}

inline int8_t sbuf_t::get8i(size_t i,sbuf_t::byte_order_t bo) const {
    return bo==BO_LITTLE_ENDIAN ? get8u(i) : get8uBE(i);
}

inline int16_t sbuf_t::get16i(size_t i,sbuf_t::byte_order_t bo) const {
    return bo==BO_LITTLE_ENDIAN ? get16u(i) : get8uBE(i);
}

inline int32_t sbuf_t::get32i(size_t i,sbuf_t::byte_order_t bo) const {
    return bo==BO_LITTLE_ENDIAN ? get32u(i) : get32uBE(i);
}

inline int64_t sbuf_t::get64i(size_t i,sbuf_t::byte_order_t bo) const {
    return bo==BO_LITTLE_ENDIAN ? get64u(i) : get64uBE(i);
}

#endif
