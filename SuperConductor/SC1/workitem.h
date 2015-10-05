// workitem.h

#ifndef WORKITEM_H
#define WORKITEM_H
#include <fstream.h>
#include<qstring.h>
#include<qdatetime.h>
#include"job.h"

class Job;

QTime operator-(QTime l,QTime r);
QTime operator+(QTime l,QTime r);
QTime operator/(QTime l,QTime r);
QTime operator*(QTime l,int r);
QTime operator/(QTime l,int r);

class WorkItem
{
   private:
      Job* job;
      QString frameInfo;
      QDateTime start;
      QDateTime finish;
   public:
      Job* getJob(){return job;}
      WorkItem(WorkItem& clone) { frameInfo = clone.frameInfo; job = clone.job;}
      WorkItem(Job* parentJob,QString name = "") { frameInfo = name; job = parentJob;}
      ~WorkItem(){/*do not delete job!*/}
      QString getFrameInfo(){ return frameInfo; }
      void setParentJob(Job* parent){job = parent;}
      QDateTime getStart();
      QDateTime getFinish();
      void setStart();
      void setFinish();
};

#endif
