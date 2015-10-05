//--------------------------------------------------------------------
//  Author: CSM SuperConductor team
//  Date: 5/15/02
//
//  workqueue.cpp
//
//  class to group and manipulate Jobs.  Work Queue has
//  a list of jobs. Each job has a priority for scheduling purposes.
//  A job corresponds to a project, or set of frames.
//
//  The Work Queue current pointer keeps track of the next Job to be
//  processed.  It should not be changed anywhere except in the nextItem
//  function.
//
//--------------------------------------------------------------------

#include "workqueue.h"
#include<qmessagebox.h>

int WorkQueue::at(uint jobID)
{
   QPtrListIterator<Job> jobIterate(jobListing);
   int index = 0;
   Job* tempJob = jobIterate.toFirst();

   while(tempJob)
   {
      //if match found return index
      if(tempJob->getJobID() == jobID)
         return index;

      //no match was found, move on
      index++;
      tempJob = ++jobIterate;
   }

   //no match in entire list, return -1
   return (-1);
}

WorkQueue::WorkQueue(uint maximumFrames, uint currentPriority)
{
   priority = currentPriority;
   frameCount = 0;
   if(maximumFrames>0)
      maxFrames = maximumFrames;
   jobListing.setAutoDelete(true);
   workHouse.setAutoDelete(true);
}

WorkQueue::WorkQueue(WorkQueue& clone)
{
   QPtrListIterator<Job> tempIt(clone.jobListing);

   priority = clone.priority;
   frameCount = clone.frameCount;
   maxFrames = clone.maxFrames;

   //deap copy WorkQueue QPtrList
   Job* tempJob = tempIt.toFirst();
   while(tempJob)
   {
      addJob(new Job(*tempJob));
      tempJob = ++tempIt;
   }

   jobListing.setAutoDelete(true);
   workHouse.setAutoDelete(true);
}

WorkQueue::~WorkQueue()
{}

WorkItem* WorkQueue::nextItem(uint currentPriority,uint* jobIDAcceptor)
{
   Job* tracker,*runner;

   //make sure the list is not empty and the priority matches this WorkQueue
   if((!jobListing.isEmpty())&&(priority == currentPriority))
   {
      //make sure current is set properly
      if(!jobListing.current())
         jobListing.first();
      runner = jobListing.current();

      //put tracker at the front of the list
      tracker = jobListing.getFirst();

      do
      {
         //make sure job has undispatched frames and "time slice" is not up
         //notice frameCount is always incremented after it is checked and starts at zero
         if((runner->length() > 0)&&(frameCount++<maxFrames))
         {
            *jobIDAcceptor = runner->getJobID();
            return(runner->getWorkItem());
         }

         //job was empty or "time slice" is up
         //go to next job of same priority
         if(!jobListing.next())
            jobListing.first();
         frameCount=0;
         runner = jobListing.current();
      }while(tracker != runner);

      //went through remainder of list of Jobs for this priority, return null;
   }

   return 0;
}

bool WorkQueue::addJob(Job* job)
{
   //mark location of current pointer
   int marker = jobListing.at();
   bool successful = false;

   //make sure job is not null and priority matches
   if((job)&&(job->getPriority() == priority))
   {
      jobListing.append(job);
      connect(job,SIGNAL(disown(WorkItem*)),this,SLOT(store(WorkItem*)));

      //connect changed signal to relay to scheduler
      connect(job, SIGNAL(changed(Job*)), this, SLOT(update(Job*)));

      successful = true;
   }

   //return to original location
   if(marker >= 0)
      jobListing.at(marker);

   return successful;
}

bool WorkQueue::removeJob(uint jobID)
{
   //mark location of current pointer
   int marker = jobListing.at();  

   int index = at(jobID);
   bool confirmedClose = false;
   Job* job;

   //make sure job is in list and remove it
   if(index >= 0)
   {
      job = jobListing.at(index);
      if(job)
      {
         //if the working que or unassigned que are not empty,
         //ask for confirmation.
         if(!job->isFinished())
         {
            confirmedClose = (QMessageBox::information(0,"SuperConductor",
                              "Unfinished jobs remain\n close project anyway?",
                              "&Continue Project", "&Close Project",0,1));
         }
         else
            confirmedClose = true;

         if(confirmedClose)
            jobListing.remove(index);
      }

      //put marker back at original location
      if((confirmedClose)&&(marker>=index))
         jobListing.at(marker-1);
      else
         jobListing.at(marker);
   }

   return confirmedClose;
}

Job* WorkQueue::getJob(uint jobID)
{
   int marker = jobListing.at();
   int index = at(jobID);
   return (jobListing.take(index));
   if(marker>=index)
      jobListing.at(marker-1);
   else
      jobListing.at(marker);
}

bool WorkQueue::renderFailed(WorkItem* item)
{
   QPtrListIterator<Job> jobIterate(jobListing);
   bool failureHandled = false;
   if(!jobIterate.isEmpty())
   {
      Job* tracker = jobIterate.toFirst();
      do
      {
         //call renderFailed function of jobs, stop if any are successful
         tracker->renderFailed(item, failureHandled);
         tracker = ++jobIterate;
      }while((tracker)&&(!failureHandled));
   }
   if(!failureHandled)
      workHouse.removeRef(item);

   return failureHandled;
}

bool WorkQueue::renderFinished(WorkItem* item)
{
   QPtrListIterator<Job> jobIterate(jobListing);
   bool frameFinished = false;
   if(!jobIterate.isEmpty())
   {
      Job* tracker = jobIterate.toFirst();
      do
      {
         //call renderFinished function of jobs, stop if any are successful
         tracker->renderFinished(item, frameFinished);
         tracker = ++jobIterate;
      }while((tracker)&&(!frameFinished));
   }
   if(!frameFinished)
      workHouse.removeRef(item);

   return frameFinished;
}

bool WorkQueue::containsJobID(uint jobID)
{
   int index = at(jobID);
   if(index >= 0)
      return true;

   //index was less than zero
   return false;
}

void WorkQueue::setMaxFrames(uint window)
{
   if(window > 0)
      maxFrames = window;
}

uint WorkQueue::getMaxFrames()
{
   return maxFrames;
}

uint WorkQueue::getPriority()
{
   return priority;
}

void WorkQueue::store(WorkItem* homeless)
{
   //add homeless to workHouse
   workHouse.append(homeless);
}

void WorkQueue::update(Job* mutated)
{
   //send signal to scheduler
   emit changed(mutated);
}
