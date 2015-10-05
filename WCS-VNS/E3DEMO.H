/*******************************************************************************
*
*  (C) Copyright 1994 Rainbow Technologies, Inc. All rights subject to the
*	Developer's agreement.
*
*
*	This software contains proprietary information which shall not be re-
*	produced or transferred to other documents and shall not be disclosed to
*	others or used for manufacturing or any other purpose without prior 
*	permission of Rainbow Technologies, Inc.
*
*
*	Description: This file contains functionprototypes and constant declarations
*		for the Eve3 interface for C language.
*******************************************************************************/


/************************************************************************
* ProcName: RBEHANDLE
*
* Purpose: To create a  handle for use with other Eve3 driver functions.
*
* Inputs: None
*
* Function Return: Handle - Eve3 handle value.
*
* Description: 
*   This function allocates a handle that is used by other Eve3 driver 
*   functions to contain context information about the currently active
*   Eve3 key.
*
*************************************************************************/
extern pascal Handle			 RBEHANDLE(void); 


/************************************************************************
* ProcName: RBEFINDFIRST
*
* Purpose: To search the ADB for an EVE3 that contains the specified 
*          developer ID.
*
* Inputs:  DID - 16 bit developer ID
*          pkt -  Eve 3 handle allocated by the RBE handle function.           
*
* Outputs: SN - ptr to a 32 value that contains the serial number of 
*               the found Eve3 key.
*
* Function Return:  16 bit status value.
*
* Description: 
*      Searches the ADB addresses for an EVE3 with the specified 
*      Developer ID. If one is found, its serial number is returned in 
*      the SN parameter.  If none is found, a status value of 
*      E3_NO_KEY_FOUND is returned.         
*
*************************************************************************/
extern pascal unsigned short int RBEFINDFIRST(unsigned short int DID,
										      unsigned long *SN,
											  Handle pkt);

/************************************************************************
* ProcName: RBEFINDNEXT
*
* Purpose:To search the ADB for the next EVE3 with the same 
*          developer ID.
*
* Inputs:  pkt -  Eve 3 handle allocated by the RBE handle function.
*
* Outputs: SN - ptr to a 32 value that contains the serial number of 
*               the found Eve3 key.
*
* Function Return:16 bit status value.
*
* Description: 
*      Searches the ADB addresses for the next EVE3 with the specified 
*      Developer ID. If one is found, its serial number is returned in 
*      the SN parameter.  If none is found, a status value of 
*      E3_NO_KEY_FOUND is returned.         
*
*************************************************************************/
extern pascal unsigned short int RBEFINDNEXT(unsigned long *SN,
 											 Handle pkt); 

/************************************************************************
* ProcName:RBEREAD
*
* Purpose: to read a value from a "found" Eve3.
*
* Inputs:  item - 16 bit value that specifies which item to read.
*          chlg -  a 16 bit value that is required if the item is a 
*                  challenge/response GPR.
*          pkt  -  Eve3 handle value from last successful find operation. 
*
* Outputs: data - ptr to 16 bit data value returned from the Eve3 key.
*
* Function Return:  16 bit status value.
*
* Description: 
*    This function reads the specified item from the current Eve3.
*    Valid item values include:
*
* 				E3_READ_ID		
* 				E3_READ_SNH		
* 				E3_READ_SNL		
* 				E3_READ_CNTR	
* 				E3_READ_VER		
* 				E3_READ_GPR thru E3_READ_GPR + 31		
* 				E3_READ_RAM thru E3_READ_RAM + 3		       
*
*************************************************************************/
extern pascal unsigned short int RBEREAD(unsigned short int item,
										 unsigned short int *data,
 										 unsigned short int chlg,
 										 Handle pkt);

/************************************************************************
* ProcName: RBEWRITE
*
* Purpose:
*
* Inputs:  item - 16 bit value that specifies which item to write.
*          data - 16 bit data value to be written to the Eve3 key.
*          chlg -  a 16 bit value that is either:
*                   - the challenge value of a challenge/response pair, or
*                   - the write password that is contained in the Eve3 key.
*          pkt  -  Eve3 handle value from last successful find operation. 
*
* Outputs: None except status return
*
* Function Return:  16 bit status value.
*
* Description: 
*    This function writes the specified item to the current Eve3. 
*    Valid item values include:
*
* 			E3_SET_GPR thru E3_SET_GPR + 31	    
*			E3_SET_RAM thru E3_SET_RAM + 3		
*
* 
*
*************************************************************************/
extern pascal unsigned short int RBEWRITE(unsigned short int item,
										  unsigned short int data,
									 	  unsigned short int chlg,
										  Handle pkt); 

/************************************************************************
* ProcName: RBEDNZ
*
* Purpose: To decrement an Eve3 counter value.
*
* Inputs:  cntr - 16 bit item value that specifies which counter to 
*                 decrement.
*          pkt  - Eve3 handle value from last successful find operation.
*
* Outputs: None except the status return.
*
* Function Return:  16 bit status value.
*
* Description:
*      This function performs a decrement of the Eve3 counter word 
*      specified by the cntr parameter. If the counter is already 
*      zero, the counter is not decremented and a status value of 
*      E3_COUNTER_ALREADY_ZERO is returned.
*
*************************************************************************/
extern pascal unsigned short int RBEDNZ(unsigned short int cntr,
										Handle pkt);

/************************************************************************
* ProcName: RBEQUERY
*
* Purpose: To exchange 32 bits of data thru the Eve3 algorithm engine.
*
* Inputs:  cell - the 16 bit item value that specifies which Eve3 
*                 algorithm descriptor to use when processing the query.
*          query - the 32 bit value that is used as input to the Eve3
*                  algorithm engine.
*          pkt  - Eve3 handle value from last successful find operation.      
*          
*
* Outputs: qryresp - ptr to 32 bit query response from the Eve3 key.
*
* Function Return:  16 bit status value.
*
* Description:
*
*************************************************************************/
extern pascal unsigned short int RBEQUERY(unsigned short int cell, 
										  unsigned long query,
										  unsigned long *qryresp,
										  Handle pkt); 

/************************************************************************
* ProcName: RBEDEVELOPERMODE
*
* This function replaces RBEDEVMODE.  
*
* Purpose: Allow extended functionality for many functions.  Please see
*    chapter 8 of the EvE3 manual for details.
*
* Inputs:  devPW1 - Developer Password 1
*          devPW2 - Developer Password 2
*		   devPW3 - Developer Password 3
*          pkt  - Eve3 handle value from last successful find operation.      
*
* Outputs: none 
*
* Function Return:  16 bit status value. Producing an error during this 
*      function will return the application to Application Mode.
*
* Note: This is a very powerful and revealing function.  Providing this function 
*    in an application will allow anybody to write to the proprietary part of the Eve3.  
*    This function is provided primarily for internal use.  Discretion is advised.
*
*	Please review chapter 8 for proper use.
*
*************************************************************************/
extern pascal unsigned short int 
RBEDEVELOPERMODE (   	unsigned short int devPW1, 
                    	unsigned short int devPW2, 
                    	unsigned short int devPW3, 
            			Handle Eve3Handle);

/************************************************************************
* ProcName: RBEKEYTYPE
*
* Purpose: Return the code for type of key currently found
*
* Inputs:  pkt  - Eve3 handle value from last successful find operation.      
*          
* Outputs: pKeyType - ptr to the code type
*               kE3_KEYTYPE_ADB( 1) - The current key is a ADB key.
*               kE3_KEYTYPE_USB( 2) - The current key is a USB key.
*
* Function Return:  E3_SUCCESS, 
*                   E3ERR_BADPKT,   - pkt or pKeyType is null.
*					E3ERR_NOTFOUND  - No key is defined. 
*
*************************************************************************/
extern pascal unsigned short int 
RBEKEYTYPE (   unsigned short int * pKeyType, 
               Handle pkt);

/************************************************************************
*  Eve3 Status codes
*************************************************************************/

#define E3_SUCCESS		0x0			/* No errors detected 				*/
#define E3ERR_DENIED    0x1			/* Access denied (GPR read/write)   */
#define E3ERR_ISZERO    0x2			/* Counter already 0 (Dec or Query) */
#define E3ERR_BADCMD    0x3			/* Bad command						*/
#define E3ERR_BADPKT    0x4			/* Corrupt or invalid data received */
#define E3ERR_ADBERR    0x10		/* Problem with ADB (time out, etc) */
#define E3ERR_NOTFOUND  0x20		/* Eve3 not found (Find First/Next) */


/************************************************************************
*  Eve3 Item codes
*************************************************************************/
#define E3_READ_ID		0x80
#define E3_READ_SNH		0x81
#define E3_READ_SNL		0x82
#define E3_READ_CNTR	0x84
#define E3_READ_VER		0xB2

#define E3_READ_GPR		0x00
#define E3_SET_GPR	    0x20
#define E3_READ_RAM		0xA0
#define E3_SET_RAM		0xA8
#define E3_QUERY		0x00
#define E3_DNZ_CNTR		0x00

/************************************************************************
*  Eve3 Item macros
*************************************************************************/
#define E3_READ_GPR_ITEM(x) (E3_READ_GPR + (x))
#define E3_SET_GPR_ITEM(x) (E3_SET_GPR + (x))
#define E3_READ_RAM_ITEM(x) (E3_READ_RAM + (x))
#define E3_SET_RAM_ITEM(x) (E3_SET_RAM + (x))
#define E3_QUERY_ITEM(x) (E3_QUERY + (x))
#define E3_COUNTER_ITEM(x) (E3_DNZ_CNTR + (x))



