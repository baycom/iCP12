#define SCL "RB2"
#define SDA "RA5"
#define RSDA "A"
#define RSDAN 5
#define LM75_ADDR 0x90  // incl. RW-bit

static char *xdev;
void i2c_init(void)
{
	char dummy[10];
	icp12(xdev,"("SCL"o)("SDA"o)",dummy,sizeof(dummy));
}
/*-------------------------------------------------------------------*/
void scl(int x) 
{
	char dummy[10];
	if (x)
		icp12(xdev,"("SCL"1)",dummy,sizeof(dummy));
	else
		icp12(xdev,"("SCL"0)",dummy,sizeof(dummy));

}
/*-------------------------------------------------------------------*/
void sda(int x) 
{
	char dummy[10];
	if (x)
		icp12(xdev,"("SDA"i)",dummy,sizeof(dummy));
//		icp12(xdev,"("SDA"o)("SDA"1)",dummy,sizeof(dummy));
	else
		icp12(xdev,"("SDA"o)("SDA"0)",dummy,sizeof(dummy));
}
/*-------------------------------------------------------------------*/
int rsda(void)
{
	char dummy[100];
	icp12(xdev,"("SDA"i)(DP"RSDA")",dummy,sizeof(dummy));
//	printf("R %s\n",dummy);
	if (dummy[5+RSDAN]=='1')
		return 1;
	else 
		return 0;
}
/*-------------------------------------------------------------------*/
void i2c_start(void)
{
        sda(1);
        scl(1);
        sda(0);
        scl(0);
}
/*-------------------------------------------------------------------*/
void i2c_stop(void)
{
        sda(0);
        scl(1);
        sda(1);
}
/*-------------------------------------------------------------------*/
void i2c_wbyte(int x)
{
        int n;
        for(n=0;n<8;n++) {
                if (x&0x80)
                        sda(1);
                else
                        sda(0);
                scl(1);
                scl(0);
                x<<=1;
        }
        sda(1);
        scl(1);
	// read ack
	int ack=rsda();
	//printf("ACK: %i\n",ack);
        scl(0);
}
/*-------------------------------------------------------------------*/
int i2c_rbyte(void)
{
        int n;
	int x=0;
        for(n=0;n<8;n++) {
                x<<=1;

                scl(1);
		if (rsda())
			x|=1;

                scl(0);

        }
        sda(0);
        scl(1);
        scl(0);
	//printf("r %i\n",x);
	return x;
}
/*-------------------------------------------------------------------*/
float lm75_read(char *device, int addr)
{
	short r;
	float t;
	xdev=device;
	i2c_init();
	i2c_start();
#if 1
	i2c_wbyte(LM75_ADDR+2*addr);
	i2c_wbyte(0); // temp register, set by default, but play safe...

	i2c_start();
#endif
	i2c_wbyte((LM75_ADDR+2*addr)|1);
	r=i2c_rbyte();

	r=(r<<8)|i2c_rbyte();
	i2c_stop();
	r=r>>7;
	t= r*0.5;;

	return t;
}
/*-------------------------------------------------------------------*/
