#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <vectornav.h>
/* Change the connection settings to your configuration. */
const char* const COM_PORT = "//dev//ttyUSB0";
const int BAUD_RATE = 115200;
const double g = 9.80665;

void asyncDataListener(
	void* sender,
	VnDeviceCompositeData* data);

int main()
{
	int i;
	VN_ERROR_CODE errorCode;
	Vn200 vn200;
	VnQuaternion attitude;
	VnVector3 acc,mag,angRate,b;
	VnMatrix3x3 c;
	c.c00=c.c11=c.c22=1;
	c.c01=c.c02=c.c10=c.c12=c.c20=c.c21=0;
	b.c0=b.c1=b.c2=0;
	
	errorCode = vn200_connect(
		&vn200,
		COM_PORT,
		BAUD_RATE);
	/* Make sure the user has permission to use the COM port. */
	if (errorCode == VNERR_PERMISSION_DENIED) {
		printf("Current user does not have permission to open the COM port.\n");
		printf("Try running again using 'sudo'.\n");
		return 0;
	}
	if (errorCode != VNERR_NO_ERROR){
		printf("Error encountered when trying to connect to the sensor.\n");
		return 0;
	}

	double avg[3];
	avg[0]=avg[1]=avg[2]=0;

	for(i = 1; i <= 100; i++){
		vn200_getQuaternion(
			&vn200,
			&attitude);

		b.c0 = 2 * (attitude.x * attitude.z - attitude.w * attitude.y);
		b.c1 = 2 * (attitude.w * attitude.x + attitude.y * attitude.z);
		b.c2 = attitude.w * attitude.w - attitude.x * attitude.x - attitude.y * attitude.y + attitude.z * attitude.z;

		c.c00 = attitude.w*attitude.w + attitude.x*attitude.x - attitude.y*attitude.y - attitude.z*attitude.z;
		c.c01 = 2*(attitude.x*attitude.y + attitude.w*attitude.z);
		c.c02 = 2*(attitude.x*attitude.z - attitude.y*attitude.w);
		c.c10 = 2*(attitude.x*attitude.y - attitude.w*attitude.z);
		c.c11 = attitude.w*attitude.w - attitude.x*attitude.x + attitude.y*attitude.y - attitude.z*attitude.z;
		c.c12 = 2*(attitude.y*attitude.z + attitude.w*attitude.x);
		c.c20 = 2*(attitude.x*attitude.z + attitude.w*attitude.y);
		c.c12 = 2*(attitude.y*attitude.z - attitude.w*attitude.x);
		c.c22 = attitude.w*attitude.w - attitude.x*attitude.x - attitude.y*attitude.y + attitude.z*attitude.z;

		vn200_setAccelerationCompensation(
			&vn200,
			c,
			b,
			true);

		errorCode= vn200_getAcceleration(
			&vn200,
			&acc);

		avg[0]+= acc.c0;
		avg[1]+= acc.c1;
		avg[2]+= acc.c2;
		usleep(10000);
	}
	avg[0]/=100;
	avg[1]/=100;
	avg[2]/=100;

	printf("Averages:	%lf %lf %lf\n",avg[0],avg[1],avg[2]);

	FILE * p = fopen("data.dat","w");

	for (i = 0; i < 100; i++) {

		vn200_getQuaternion(
			&vn200,
			&attitude);

		b.c0 = 2 * (attitude.x * attitude.z - attitude.w * attitude.y) + avg[0];
		b.c1 = 2 * (attitude.w * attitude.x + attitude.y * attitude.z) + avg[1];
		b.c2 = attitude.w * attitude.w - attitude.x * attitude.x - attitude.y * attitude.y + attitude.z * attitude.z + avg[2];

		c.c00 = attitude.w*attitude.w + attitude.x*attitude.x - attitude.y*attitude.y - attitude.z*attitude.z;
		c.c01 = 2*(attitude.x*attitude.y + attitude.w*attitude.z);
		c.c02 = 2*(attitude.x*attitude.z - attitude.y*attitude.w);
		c.c10 = 2*(attitude.x*attitude.y - attitude.w*attitude.z);
		c.c11 = attitude.w*attitude.w - attitude.x*attitude.x + attitude.y*attitude.y - attitude.z*attitude.z;
		c.c12 = 2*(attitude.y*attitude.z + attitude.w*attitude.x);
		c.c20 = 2*(attitude.x*attitude.z + attitude.w*attitude.y);
		c.c12 = 2*(attitude.y*attitude.z - attitude.w*attitude.x);
		c.c22 = attitude.w*attitude.w - attitude.x*attitude.x - attitude.y*attitude.y + attitude.z*attitude.z;

		vn200_setAccelerationCompensation(
			&vn200,
			c,
			b,
			true);

		errorCode= vn200_getAcceleration(
			&vn200,
		 	&acc);

		printf("IMU Solution:\n"
			"  Acceleration.x:         %+#7.2f\n"
			"  Acceleration.y:         %+#7.2f\n"
			"  Acceleration.z:         %+#7.2f\n",
			acc.c0,
			acc.c1,
			acc.c2 - g);

		usleep(10000);
		fprintf(p,"%d %lf %lf %lf\n", i, acc.c0, acc.c1, acc.c2 - g);
	}
	fclose(p);
	errorCode = vn200_disconnect(&vn200);
	if (errorCode != VNERR_NO_ERROR){
	printf("Error encountered when trying to disconnect from the sensor.\n");
	return 0;
	}
	return 0;
}
