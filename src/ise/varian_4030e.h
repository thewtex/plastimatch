/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _varian_4030e_h_
#define _varian_4030e_h_

#include "ise_config.h"
#include <windows.h>
//#include <QMutex>
#include "HcpErrors.h"
#include "HcpFuncDefs.h"
#include "iostatus.h"
#include "acquire_4030e_child.h"
#include <vector>

#define HCP_SAME_IMAGE_ERROR -115 

class Dips_panel;

struct IMGINFO
{
	double meanVal;
	double SD;
	double minVal;
	double maxVal;
};



class Varian_4030e {
public:
    Varian_4030e (int idx);
    Varian_4030e (int idx, Acquire_4030e_child* pParent);
    ~Varian_4030e ();

  //  static QMutex vip_mutex;
    static const char* error_string (int error_code);

    int open_link (int panelIdx, const char *path);
    int check_link(int MaxTryCnt);
    void close_link ();
    void print_sys_info ();
    int get_mode_info (SModeInfo &modeInfo, int current_mode);
    int print_mode_info ();
    int query_prog_info (UQueryProgInfo &crntStatus, bool showAll = FALSE);
    int wait_on_complete (UQueryProgInfo &crntStatus, int timeoutMsec = 0);
    int wait_on_num_frames (
        UQueryProgInfo &crntStatus, int numRequested, int timeoutMsec = 0);
    int wait_on_num_pulses (UQueryProgInfo &crntStatus, int timeoutMsec = 0);
    int wait_on_ready_for_pulse (UQueryProgInfo &crntStatus, 
        int timeoutMsec = 0, int expectedState = TRUE);
    //int rad_acquisition (Dips_panel *dp);
    int perform_sw_rad_acquisition ();
    int get_image_to_file (int xSize, int ySize, 
	char *filename, int imageType = VIP_CURRENT_IMAGE);

	int get_image_to_buf (int xSize, int ySize);
    int get_image_to_dips (Dips_panel *dp, int xSize, int ySize);

	bool CopyFromBufAndSendToDips (Dips_panel *dp); //get cur image to curImage
	void CalcImageInfo (double& meanVal, double& STDV, double& minVal, double& maxVal, int sizeX, int sizeY, USHORT* image_ptr); //all of the images taken before will be inspected
	bool SameImageExist(IMGINFO& curInfo, int& sameImgIndex);

    int disable_missing_corrections (int result);

	//int Audit_All_Image (int xSize, int ySize); //get cur image to curImage

public:
    int idx;
    int current_mode;
    int receptor_no;

    Acquire_4030e_child* m_pParent;
    int SelectReceptor ();

    int m_iSizeX;
    int m_iSizeY;
    //YK custom variables
    //bool m_bDarkCorrApply;
    //bool m_bGainCorrApply;

	std::vector<IMGINFO> m_vImageInfo;
};

int CheckRecLink();

#endif
