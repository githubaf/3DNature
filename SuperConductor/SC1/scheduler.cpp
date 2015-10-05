// scheduler.cpp

#include"scheduler.h"

Scheduler::Scheduler(uint maximumPriority)
{
   WorkQueue* temp;
   uint timeSlice;

   if(maximumPriority > 0)
      maxPriority=maximumPriority;

   //initialize priorityList
   for(currentPriority=0;currentPriority<=maxPriority+1;currentPriority++)
   {
      timeSlice = maximumPriority +1 -currentPriority;
      temp = new WorkQueue(timeSlice,currentPriority);
      connect(temp,SIGNAL(changed(Job*)),this,SLOT(update(Job*)));
      priorityList.append(temp);
   }

   currentPriority=1;
}

Scheduler::Scheduler(Scheduler& clone)
{
   WorkQueue* temp,*tempCopy;
   maxPriority = clone.maxPriority;

   //deep copy priority list
   for(currentPriority=0;currentPriority<=maxPriority+1;currentPriority++)
   {
      temp = clone.priorityList.at(currentPriority);
      if(temp)
      {
         tempCopy = new WorkQueue(*temp);
         connect(tempCopy,SIGNAL(changed(Job*)),this,SLOT(update(Job*)));
         priorityList.append(tempCopy);
      }
   }
   currentPriority = clone.currentPriority;
}

Scheduler::~Scheduler()
{
   priorityList.setAutoDelete(true);
}

WorkItem* Scheduler::getItem(uint* jobIDAcceptor)
{
   uint tracker;
   WorkItem* itemPtr;
   WorkQueue* jobs;

   //make sure that currentPriority is set properly
   if((currentPriority > maxPriority)||(currentPriority < 1))
      currentPriority=1;

   //check zero priority
   jobs = priorityList.at(0);
   if(jobs)
   {
      itemPtr = jobs->nextItem(0,jobIDAcceptor);
      if(itemPtr)
         return itemPtr;

      //try again in case it stopped due to frame limit or end of job list
      itemPtr = jobs->nextItem(0,jobIDAcceptor);
      if(itemPtr)
         return itemPtr;
   }

   //remember what the current priority is
   tracker = currentPriority;

   do
   {
      //get WorkQueue for current priority
      jobs = priorityList.at(currentPriority);

      //if there is a WorkQueue for that priority get WorkItem
      if(jobs)
      {
         itemPtr = jobs->nextItem(currentPriority,jobIDAcceptor);
         if(itemPtr)
            return itemPtr;
      }

      //no work item found for that priority
      //increment currentPriority or loop back to 1
      if(++currentPriority > maxPriority)
         currentPriority=1;
   }while(currentPriority != tracker);

   //returned to the original priority and could not find a work item
   //check the original priority to see if it has any jobs
      //it may have reached its time slice and given other Queues a chance

   jobs = priorityList.at(tracker);
   if(jobs)
      itemPtr = jobs->nextItem(tracker,jobIDAcceptor);

   if(!itemPtr)
   {
      //exhausted all other possibilites, check max + 1 priority
      jobs = priorityList.at(maxPriority+1);
      if(jobs)
         itemPtr = jobs->nextItem(maxPriority+1,jobIDAcceptor);
      if(!itemPtr)
         itemPtr = jobs->nextItem(maxPriority+1,jobIDAcceptor); //try again in case of frame limit
   }

   return itemPtr;
}

void Scheduler::renderFailed(WorkItem* item)
{
   WorkQueue* temp = priorityList.first();
   bool containsItem = false;
   if(item)
   {
      while((temp)&&(!containsItem))
      {
         containsItem = temp->renderFailed(item);
         temp = priorityList.next();
      }
   }
}

void Scheduler::renderFinished(WorkItem* item)
{
   WorkQueue* temp = priorityList.first();
   bool containsItem = false;
   if(item)
   {
      while((temp)&&(!containsItem))
      {
         containsItem = temp->renderFinished(item);
         temp = priorityList.next();
      }
   }
}

bool Scheduler::addJob(Job* job)
{
   bool successful = false;
   WorkQueue* jobQue;
   uint jobPriority = job->getPriority();
   uint jobID = job->getJobID();

   if((job)&&(jobPriority <= maxPriority+1))
   {
      //make sure that the Job ID is unique
      jobQue = priorityList.first();
      while(jobQue)
      {
         //if job already exists with same ID return false
         if(jobQue->containsJobID(jobID))
            return successful;                 //successful is false here
         jobQue = priorityList.next();
      }

      //no job found with the same job ID
      //get WorkQueue for priority of job
      jobQue = priorityList.at(jobPriority);
      if(jobQue)
         successful = jobQue->addJob(job);
   }
   return successful;
}

bool Scheduler::endJob(uint jobID)
{
   WorkQueue* tempQue = priorityList.first();
   bool jobDeleted = false;
   while((tempQue)&&(!jobDeleted))
   {
      jobDeleted = tempQue->removeJob(jobID);
      tempQue = priorityList.next();
   }
   return jobDeleted;
}

void Scheduler::setPriority(uint jobID, uint priorityValue)
{
   Job* movingJob;
   WorkQueue* tempQue,*jobQue = priorityList.first();
   while(jobQue)                                    //search for WorkQueue containing Job with jobID
   {
      movingJob = jobQue->getJob(jobID);            //removes job without deleting it
      if(movingJob)
      {                                             //found WorkQueue containing Job with jobID
         tempQue = priorityList.at(priorityValue);  //find WorkQueue of new priority
         if(tempQue)
         {
            movingJob->setPriority(priorityValue);  //found new WorkQueue, put job into it
            tempQue->addJob(movingJob);
         }
         else
            jobQue->addJob(movingJob);             //priority not valid, put Job back in original WorkQueue
         return;                                   //found job, so quit
      }
      jobQue = priorityList.next();
   }
}

QString Scheduler::getJobName(uint jobID)
{
   //search for WorkQueue containing Job with jobID
   Job* tempJob;
   WorkQueue* tempQue = priorityList.first();
   while(tempQue)
   {
      tempJob = tempQue->getJob(jobID);
      if(tempJob)
      {
         //found Job containin jobID (getting pointer removes job from list so have to put it back)
         tempQue->addJob(tempJob);
         return (tempJob->getJobName());
      }
      tempQue = priorityList.next();
   }
   //did not find Job with jobID
   return (QString("No Job With Matching ID"));
}

uint Scheduler::getMaxPriority()
{
   return maxPriority;
}

uint Scheduler::getCurrentPriority()
{
   return currentPriority;
}

void Scheduler::update(Job* mutated)
{
   //pass signal on to interface
   emit changed(mutated);
}
