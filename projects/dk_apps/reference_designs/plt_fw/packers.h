/**
 ****************************************************************************************
 *
 * @file packers.h
 *
 * @brief Help functions
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef PACKERS_H_
#define PACKERS_H_

__STATIC_INLINE uint8_t r8le(uint8_t *p)
{
	return (uint8_t) p[0];
}

__STATIC_INLINE uint16_t r16le(uint8_t *p)
{
	return (uint16_t) (p[0] | (p[1] << 8));
}

__STATIC_INLINE void w8le(uint8_t *p, uint8_t v)
{
	p[0] = v;
}

__STATIC_INLINE void w16le(uint8_t *p, uint16_t v )
{
	p[0] = v & 0xff;
	p[1] = (v >> 8) & 0xff;
}

#define padvN(p, N) \
	p += (N)

#define padv(ptr, type) \
	padvN(ptr, sizeof(type))


#endif /* PACKERS_H_ */
