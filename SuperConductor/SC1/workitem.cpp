// workitem.cpp

#include "workitem.h"

QTime operator-(QTime l,QTime r)
{
	int h=l.hour()-r.hour(),m=l.minute()-r.minute(),s=l.second()-r.second(),ms=l.msec()-r.msec();
	if (ms<0) {ms=ms+1000;s--;}
	if (s<0) {s+=60;m--;}
	if (m<0) {m+=60;h--;}
	if (h<0) {h+=24;}
	QTime re(h,m,s,ms);
	
	return re;
}

QTime operator+(QTime l,QTime r)
{
	int h=l.hour()+r.hour(),m=l.minute()+r.minute(),s=l.second()+r.second(),ms=l.msec()+r.msec();
	if (ms>=1000) {ms=ms-1000;s++;}
	if (s>=60) {s-=60;m++;}
	if (m>=60) {m-=60;h++;}
        if (h>=24) {h-=24;}
	QTime re(h,m,s,ms);
	
	return re;
}

QTime operator/(QTime l,QTime r)
{
        //convert everything into milliseconds, devide, and convert back
        int left=l.hour()*60*60*1000 + l.minute()*60*1000 + l.second()*1000 + l.msec();
        int right=r.hour()*60*60*1000 + r.minute()*60*1000 + r.second()*1000 + r.msec();
        if (right==0)
           return QTime(0,0,0,0);
        int result = left/right;
        int ms = result%1000;
        result = (result - ms)/1000;
        int s = result%60;
        result = (result - s)/60;
        int m = result%60;
        result = (result - m)/60;
        int h = result;
	QTime re(h,m,s,ms);
	return re;
}

QTime operator/(QTime l,int r)
{
        //convert everything into milliseconds, devide, and convert back
        int left=l.hour()*60*60*1000 + l.minute()*60*1000 + l.second()*1000 + l.msec();
        int result = left/r;
        int ms = result%1000;
        result = (result - ms)/1000;
        int s = result%60;
        result = (result - s)/60;
        int m = result%60;
        result = (result - m)/60;
        int h = result;
	QTime re(h,m,s,ms);
	return re;
}

QTime operator*(QTime l,int r)
{
	int left=l.hour()*60*60*1000 + l.minute()*60*1000 + l.second()*1000 + l.msec();
	int result=left*r;
	int ms = result%1000;
	result = (result - ms)/1000;
	int s = result%60;
	result = (result - s)/60;
	int m = result%60;
	result = (result - m)/60;
	int h = result;
	QTime re;
	if (h<24) re.setHMS(h,m,s,ms);
	else re.setHMS(0,0,0,0);
	return re;
}

QDateTime WorkItem::getStart()
{
   return start;
}

QDateTime WorkItem::getFinish()
{
   return finish;
}

void WorkItem::setStart()
{
   QTime time;
   time.start();
   QDate date = QDate::currentDate();
   start = QDateTime(date,time);
}

void WorkItem::setFinish()
{
   QTime time;
   time.start();
   QDate date = QDate::currentDate();
   finish = QDateTime(date,time);
}
