/**
 * @file XTimes.h
 * @since 2022/4/24
 */

#ifndef INCLUDE_BASE_XTIMES_H_
#define INCLUDE_BASE_XTIMES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "XTypes.h"
/*
 * XTimes.c
 *
 *  Created on: 2022年4月24日
 *      Author: Administrator
 */

#include <time.h>
#include <sys/time.h>

/**
 * @brief 转换并返回tm结构的时间
 *
 * @param       unixSecs    number of seconds since 1970-01-01 00:00:00 +0000 (UTC)
 * @param[out]  pTm         用于输出当前时间的数据缓存
 * @param       timeZone    相对于标准时间的时区差, 例如北京时间的时区为 8
 * @return      tm结构的时间
 * @see         http://www.cnblogs.com/westfly/p/5139645.html
 */
static __inline struct tm*
__stimetodate(const long long unixSecs, struct tm *pResult, int timeZone) {
#   define  K_HOURS_IN_DAY          (24)
#   define  K_MINUTES_IN_HOUR       (60)
#   define  K_DAYS_FROM_UNIXTIME    (2472632)
#   define  K_DAYS_FROM_YEAR        (153)
#   define  K_MAGIC_UNKONWN_FIRST   (146097)
#   define  K_MAGIC_UNKONWN_SEC     (1461)

	int hours;
	int a, b, c, d, e, m;

	pResult->tm_sec = unixSecs % K_MINUTES_IN_HOUR;

	hours = (int) (unixSecs / K_MINUTES_IN_HOUR);
	pResult->tm_min = hours % K_MINUTES_IN_HOUR;

	hours /= K_MINUTES_IN_HOUR;
	pResult->tm_hour = (hours + timeZone) % K_HOURS_IN_DAY;
	pResult->tm_mday = (hours + timeZone) / K_HOURS_IN_DAY;

	a = pResult->tm_mday + K_DAYS_FROM_UNIXTIME;
	b = (a * 4 + 3) / K_MAGIC_UNKONWN_FIRST;
	c = (-b * K_MAGIC_UNKONWN_FIRST) / 4 + a;
	d = ((c * 4 + 3) / K_MAGIC_UNKONWN_SEC);
	e = -d * K_MAGIC_UNKONWN_SEC;
	e = e / 4 + c;

	m = (5 * e + 2) / K_DAYS_FROM_YEAR;
	pResult->tm_mday = -(K_DAYS_FROM_YEAR * m + 2) / 5 + e + 1;
	pResult->tm_mon = (-m / 10) * 12 + m + 2;
	pResult->tm_year = b * 100 + d - 6700 + (m / 10);

	return (pResult);
}

/**
 * @brief 获取毫秒时间HHMMSSsss
 * 
 * @return XInt HHMMSSsss
 */
static inline XInt XGetComMSec() {
	XInt iusec = 0;
	struct timeval tv;
	struct tm tm1;

	memset(&tv, 0, sizeof(struct timeval));
	gettimeofday(&tv, NULL);

	__stimetodate(tv.tv_sec, &tm1, 8);

	iusec = (tm1.tm_hour * 10000 + tm1.tm_min * 100 + tm1.tm_sec) * 1000
			+ tv.tv_usec / 1000;

	return (iusec);
}
/**
 * @brief 返回毫秒的clocktime
 * 
 * @return XInt 
 */
static inline XInt XGetStandMSec() {
	XInt us = 0;
	struct timeval t = { 0, 0 }, t2 = { 0, 0 };
	struct tm tm1;

	gettimeofday(&t, NULL);
	t2.tv_sec = t.tv_sec;
	t2.tv_usec = t.tv_usec;

	__stimetodate(t2.tv_sec, &tm1, 8);

	us = (tm1.tm_hour * 3600 + tm1.tm_min * 60 + tm1.tm_sec) * 1000
			+ t2.tv_usec / 1000;

	return (us);
}

/**
 * @brief 返回毫秒的clocktime
 * 
 * @return XLong 
 */
static inline XLong XGetMSec() {
	XLong ms;
	struct timeval t = { 0, 0 };

	gettimeofday(&t, NULL);
	ms = t.tv_sec * 1000 + t.tv_usec / 1000;

	return (ms);
}

/**
 * @brief 返回微秒的clocktime
 * 
 * @return XLong 
 */
static inline XLong XGetUSec() {
	XLong us;
	struct timeval t = { 0, 0 };

	gettimeofday(&t, NULL);

	us = t.tv_sec * 1000000 + t.tv_usec;

	return (us);
}

/**
 * @brief 返回微秒的clocktime
 * 
 * @return XDouble 
 */
static inline XDouble XGetStandUSec() {
	XDouble us = 0.0;
	struct timeval t = { 0, 0 }, t2 = { 0, 0 };
	struct tm tm1;

	gettimeofday(&t, NULL);
	t2.tv_sec = t.tv_sec;
	t2.tv_usec = t.tv_usec;

	__stimetodate(t2.tv_sec, &tm1, 8);

	us = (tm1.tm_hour * 3600 + tm1.tm_min * 60 + tm1.tm_sec)
			+ t2.tv_usec * 0.000001;

	return (us);
}

/**
 * @brief 返回微秒的时间HHMMSSsss.uuu
 * 
 * @return XDouble 
 */
static inline XDouble XGetComUSec() {
	XDouble us = 0.0;
	struct timeval t = { 0, 0 }, t2 = { 0, 0 };
	struct tm tm1;

	gettimeofday(&t, NULL);
	t2.tv_sec = t.tv_sec;
	t2.tv_usec = t.tv_usec;

	__stimetodate(t2.tv_sec, &tm1, 8);
	us = tm1.tm_hour * 10000 + tm1.tm_min * 100 + tm1.tm_sec
			+ t2.tv_usec * 0.000001;

	return (us);
}

static inline int64_t host_utc_sec_offset() {
  time_t secs = (time_t)time(NULL);
  struct tm tm_;
  localtime_r(&secs, &tm_);
  return ((int64_t)tm_.tm_gmtoff);
}

/**
 * @brief 获取时钟时间
 * 
 * @return XLLong 
 */
static inline XLLong XGetClockTime() {
	XLLong ns;
	struct timespec t = { 0, 0 };
	XInt iret = -1;

	//###精度最高，但是它系统调用的开销大
	iret = clock_gettime(CLOCK_REALTIME, &t);
	if(iret == -1)
	{
		 perror("clock_gettime"); // 打印错误信息
	}
	ns =  ((XLLong)t.tv_sec  * 1000000000LL + t.tv_nsec);

	return (ns);
}

/**
 * @brief 获取时钟时间
 * 
 * @param tTime 
 * @return XVoid 
 */
static inline  XVoid XGetSClockTime(XChar* tTime) {
	struct timespec t = { 0, 0 };
	//###精度最高，但是它系统调用的开销大
	clock_gettime(CLOCK_REALTIME, &t);

	sprintf(tTime, "%lld", (XLLong)t.tv_sec * 1000000000LL + t.tv_nsec);
}

/**
 * @brief 转换纳秒时钟时间为HHMMSS.sssuuu
 * 
 * @param ns 
 * @return XDouble 
 */
static inline  XDouble XNsTime2D(XLLong ns) {
	XDouble nd = 0.0;
	XLong nano = ns % 1000000000LL;
	XLong nsec = ns / 1000000000LL;

	XInt secs = nsec % 60;
	XInt mins = nsec / 60 % 60;
	XInt hours = (nsec / 3600 % 24 + 8) % 24;

	nd = hours*10000 + mins * 100 + secs + (XDouble)nano / 1000000000LL;

	return (nd);
}

/**
 * @brief 将ns时间转为格式化的ms时间
 * 
 * @param ns 
 * @return XInt 
 */
static inline  XInt XNsTime2I(XLLong ns)
{
	XInt ms = 0;

	XLong nano = ns % 1000000000LL;
	XLong nsec = ns / 1000000000LL;

	XInt secs = nsec % 60;
	XInt mins = nsec / 60 % 60;
	XInt hours = (nsec / 3600 % 24 + 8) % 24;

	ms = nano / 1000000;
	ms += (hours*10000 + mins * 100 + secs) * 1000;

	return (ms);
}

/**
 * @brief 将纳秒时钟时间转为HH:MM:SS:sss的字符串
 * 
 * @param ns 
 * @param result 
 * @return XChar* 
 */
static inline XChar* XNsTime2S(XLLong ns, XChar* result) {
//	XDouble nd = 0.0;
//	XLLong nss = ns;

	XLong nano = ns % 1000000000LL;
	XLong nsec = ns / 1000000000LL;

	XInt secs = nsec % 60;
	XInt mins = nsec / 60 % 60;
	XInt hours = (nsec / 3600 % 24 + 8) % 24;

	sprintf(result, "%02d:%02d:%02d:%ld", hours, mins, secs, nano);

	return (result);
}

/**
 * @brief 将clock时间转为HHMMSS.sss
 * 
 * @param ntime 
 * @return XDouble 
 */
static inline XDouble XLTime2D(XLLong ntime) {
	XDouble dtime;
	XInt hour, min, sec, msec;

	ntime = ntime % 10000000; //去除年月日

	hour = ntime / 10000000;
	min = ntime % 10000000 / 100000;
	sec = ntime % 100000 / 1000;
	msec = ntime % 1000;
	dtime = hour * 3600 + min * 60 + sec + msec * 0.001;

	return (dtime);
}

/**
 * @brief 将标准时间转换为HHMMSSsss
 * 
 * @param msec 
 * @return XInt 
 */
static inline XInt XMsS2C(XInt msec) {
	XInt ms = 0;
	XInt h = 0, m = 0, s = 0, u = 0;
	u = msec % 1000;
	h = msec / 1000 / 3600;
	m = msec / 1000 % 3600 / 60;
	s = msec / 1000 % 3600 % 60;
	ms = (h * 10000 + m * 100 + s) * 1000 + u;

	return (ms);
}

/**
 * @brief 将格式时间HHMMSSsss转换为标准时间
 * 
 * @param msec 
 * @return XInt 
 */
static inline XInt XMsC2S(XLLong msec) {
	XInt ms = 0;
	XInt h = 0, m = 0, s = 0, u = 0;
	u = msec % 1000;
	h = msec / 1000 / 10000;
	m = msec / 1000 % 10000 / 100;
	s = msec / 1000 % 10000 % 100;
	ms = (h * 3600 + m * 60 + s) * 1000 + u;

	return (ms);
}

/**
 * @brief us等待时间
 * 
 * @param us 
 * @return XVoid 
 */
static inline XVoid XUSleep(XInt us) {
	struct timespec __SLEEP_US_ts = { 0, 0 };
	long __SLEEP_US_msec = (us);
	if (__SLEEP_US_msec < 1000000) {
		__SLEEP_US_ts.tv_nsec = __SLEEP_US_msec * 1000;
	} else {
		__SLEEP_US_ts.tv_sec = (__SLEEP_US_msec / 1000000);
		__SLEEP_US_ts.tv_nsec = (__SLEEP_US_msec % 1000000) * 1000;
	}
	nanosleep(&__SLEEP_US_ts, (struct timespec*) NULL);
}

/**
 * @brief 获取当前时间HHMMSS
 * 
 * @return XInt 
 */
static inline XInt XGetCurTime() {
	XInt itime = 0;
	struct timeval tv;
	struct tm tm1;

	memset(&tv, 0, sizeof(struct timeval));
	gettimeofday(&tv, NULL);

	__stimetodate(tv.tv_sec, &tm1, 8);

	itime = tm1.tm_hour * 10000 + tm1.tm_min * 100 + tm1.tm_sec;

	return (itime);
}

static inline XInt XDiffDay(XInt iBeginDate, XInt iEndDate) {
	XInt beginYear = 0;
	XInt beginMonth = 0;
	XInt beginDay = 0;
	XInt endYear = 0;
	XInt endMonth = 0;
	XInt endDay = 0;
	XInt y1 = 0;
	XInt m1 = 0;
	XInt d1 = 0;
	XInt y2 = 0;
	XInt m2 = 0;
	XInt d2 = 0;

	beginYear = iBeginDate / 10000;
	beginMonth = (iBeginDate % 10000) / 100;
	beginDay = iBeginDate % 100;

	endYear = iEndDate / 10000;
	endMonth = (iEndDate % 10000) / 100;
	endDay = iEndDate % 100;

	m1 = (beginMonth + 9) % 12;
	y1 = beginYear - m1 / 10;
	d1 = 365 * y1 + y1 / 4 - y1 / 100 + y1 / 400 + (m1 * 306 + 5) / 10
			+ (beginDay - 1);

	m2 = (endMonth + 9) % 12;
	y2 = endYear - m2 / 10;
	d2 = 365 * y2 + y2 / 4 - y2 / 100 + y2 / 400 + (m2 * 306 + 5) / 10
			+ (endDay - 1);

	return (d2 - d1);
}

/**
 * @brief 获取今天的日期YYYYMMDD
 * 
 * @return XInt 
 */
static inline XInt XGetToday() {
	time_t rawtime;
	struct tm *ptminfo;
	time(&rawtime);
	ptminfo = localtime(&rawtime);

	return ((ptminfo->tm_year + 1900) * 10000 + (ptminfo->tm_mon + 1) * 100
			+ (ptminfo->tm_mday));
}


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_BASE_XTIMES_H_ */
