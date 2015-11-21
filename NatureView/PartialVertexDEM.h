// PartialVertexDEM.h


#ifndef NVW_PARTIALVERTEXDEM_H
#define NVW_PARTIALVERTEXDEM_H

class MiniVertexDEM
	{
	public:
		float XYZ[3], xyz[3], Elev;
	}; // MiniVertexDEM

class PartialVertexDEM
	{
	public:
		double Lat, Lon, Elev;
		float XYZ[3], xyz[3];
	}; // PartialVertexDEM


#endif // !NVW_PARTIALVERTEXDEM_H
