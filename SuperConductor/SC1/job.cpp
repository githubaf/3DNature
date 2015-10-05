//--------------------------------------------------------------------
//  Author: CSM SuperConductor team
//  Date: 5/15/02
//
//  job.cpp
//
//  class to group and manipulate Work Items.  Work Queue will have
//  a list of jobs. Each job has a priority for scheduling purposes.
//
//--------------------------------------------------------------------

#include"job.h"

Job::Job(uint priorityValue, uint IDValue, QString name, QString file)
{
   priority = priorityValue;
   jobID = IDValue;
   jobName = name;
   fileName = file;

   finished.setAutoDelete(true);
   working.setAutoDelete(true);
   unassigned.setAutoDelete(true);
}

Job::Job(Job& clone)
{
   priority = clone.priority;
   jobName = clone.jobName;
   fileName = clone.fileName;
   jobID = clone.jobID;
   start=clone.start;
   finish=clone.finish;
   
   WorkItem* tempOld;

   //deep copy unassigned QPtrList
   tempOld = clone.unassigned.first();
   while(tempOld)
   {
      unassigned.append(new WorkItem(this,tempOld->getFrameInfo()));
      tempOld = clone.unassigned.next();
   }

   //deep copy working QPtrList
   tempOld = clone.working.first();
   while(tempOld)
   {
      working.append(new WorkItem(this,tempOld->getFrameInfo()));
      tempOld = clone.working.next();
   }

   //deep copy finished QPtrList
   tempOld = clone.finished.first();
   while(tempOld)
   {
      finished.append(new WorkItem(this,tempOld->getFrameInfo()));
      tempOld = clone.finished.next();
   }

   finished.setAutoDelete(true);
   working.setAutoDelete(true);
   unassigned.setAutoDelete(true);
}

Job::~Job()
{
   WorkItem* tempItem;
   working.first();
   while(!working.isEmpty())
   {
      tempItem = working.take();

      //set workitems job pointer to null
      tempItem->setParentJob(0);

      //put work items in a special list of WorkQueue class to be deleted
      //when they are finished.
      emit disown(tempItem);
   }
}

bool Job::inList(Que* list, QString itemInfo)
{
   if(list)
   {
      //traverse list from front
      WorkItem* listWalker = list->first();
      while(listWalker)
      {
         //return true if ID matches
         if(listWalker->getFrameInfo() == itemInfo)
            return true;
         listWalker = list->next();
      }
   }
   //did not find matching ID in list
   return false;
}

void Job::add(QString itemInfo)
{
   //make sure that the WorkItem info is unique within Job
   if(inList(&unassigned,itemInfo)||inList(&working,itemInfo)||inList(&finished,itemInfo))
      return;

   //the info is unique, make workItem and add it to the list
   unassigned.append(new WorkItem(this,itemInfo));
}

uint Job::length()
{
   return unassigned.count();
}

uint Job::totalLength()
{
   return (unassigned.count() + working.count() + finished.count());
}

WorkItem* Job::getWorkItem()
{
   uint size = unassigned.count();
   WorkItem* temp = 0;
   if(size > 0)
   {
      //set start time if first frame being dispatched
      if((working.count()==0) && (finished.count()==0))
         setStart();

      //move item to working queue
      temp = unassigned.take(0);
      temp->setStart();
      working.append(temp);

      //let interface/scheduler know that status changed
      emit changed(this);
   }
   return temp;
}

uint Job::getPriority()
{
   return priority;
}

uint Job::getJobID()
{
   return jobID;
}

QString Job::getJobName()
{
   return jobName;
}

QString Job::getFileName()
{
   return fileName;
}

QTime Job::getStart()
{
   return start.time();
}

QTime Job::getFinish()
{
   return finish.time();
}

bool Job::isWorking()
{
   return (!working.isEmpty());
}

bool Job::isFinished()
{
   if ((working.count() == 0)&&(unassigned.count() == 0))
      return true;
   return false;
}

void Job::setPriority(uint priVal)
{
   priority = priVal;
}

void Job::setStart()
{
   QTime time;
   time.start();
   QDate date = QDate::currentDate();
   start = QDateTime(date,time);
}

void Job::setFinish()
{
   QTime time;
   time.start();
   QDate date = QDate::currentDate();
   finish = QDateTime(date,time);
}

QTime Job::getEstimatedTime(int &days)
{
   QTime current,elapsed;

   //[(current time - start time)/frames completed] * frames left
   if (finished.count()>0)
   {
      current.start();
      elapsed=current-getStart();
      elapsed=(elapsed/finished.count())*(unassigned.count()+working.count());
      return elapsed;
   }
   return elapsed;
}

QTime Job::avgRenderTime(int& days)
{
   QTime sum,diff,midnight;
   QDateTime left,right;
   int tempDays,counter = 0,hour;

   //24*60*60*1000 = 86400000
   const long MS_PER_DAY = 86400000;

   long daySeconds, timeSeconds;
   WorkItem* currentItem = finished.first();
   days = 0;

   //traverse finished list
   while(currentItem)
   {
      left = currentItem->getFinish();
      right = currentItem->getStart();
      diff = left.time() - right.time();

      //if the hours combine to more than 24 add a day
      hour = diff.hour() + sum.hour();
      if(hour >= 24)
         days++;

      //add to sum
      sum = diff + sum;

      //find the days between start time and finish time
      tempDays = right.daysTo(left);

      //if the difference is greater than one day add the difference
      if(tempDays > 0)
      {
         days = days + tempDays - 1;
         if(diff >= midnight)
            days++;
      }

      currentItem = finished.next();
      counter++;
   }

   //now calculate the average.
   //days must be converted into ms
   //sum must be converted into ms
   //the two must be added and devided by counter
   daySeconds = ((double)days/counter)*24*60*60*1000;
   timeSeconds = sum.hour()*60*60*1000 + sum.minute()*60*1000 + sum.second()*1000 + sum.msec();

   if(counter > 0)
      timeSeconds = timeSeconds/counter + daySeconds;
   else
      timeSeconds = 0;

   midnight = midnight.addMSecs(timeSeconds%MS_PER_DAY);

   days = timeSeconds/MS_PER_DAY;
   return midnight;
}

void Job::renderFailed(WorkItem* item, bool& itemInJob)
{
   int index = -1;
   //find item in working list
   if(item)
      index = working.findRef(item);

   //move to the unassigned list
   if(index >= 0)
   {
      unassigned.prepend(working.take(index));
      itemInJob = true;

      //let interface/scheduler know that status changed
      emit changed(this);
   }
}

void Job::renderFinished(WorkItem* item, bool& itemInJob)
{
   WorkItem* temp = 0;
   int index = -1;
   //find item in working list
   if(item)
      index = working.findRef(item);

   //move to the finished list
   if(index >= 0)
   {
      temp = working.take(index);
      temp->setFinish();
      finished.append(temp);
      itemInJob = true;

      //if last frame finished rendering set the finish time
      if(isFinished())
         setFinish();

      //emit signal to let interface/scheduler know that status changed
      emit changed(this);
   }
}
