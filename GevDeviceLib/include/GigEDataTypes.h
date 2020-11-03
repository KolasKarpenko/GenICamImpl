#ifndef INCLUDED_opstr_c_GigEDataTypes_h
#define INCLUDED_opstr_c_GigEDataTypes_h



#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#  define OPSTR_OS_DARWIN
#  define OPSTR_OS_BSD4
#  ifdef __LP64__
#    define OPSTR_OS_DARWIN64
#  else
#    define OPSTR_OS_DARWIN32
#  endif
#elif defined(__CYGWIN__)
#  define OPSTR_OS_CYGWIN
#elif defined(MSDOS) || defined(_MSDOS)
#  define OPSTR_OS_MSDOS
#elif defined(__OS2__)
#  if defined(__EMX__)
#    define OPSTR_OS_OS2EMX
#  else
#    define OPSTR_OS_OS2
#  endif
#elif !defined(SAG_COM) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#  define OPSTR_OS_WIN64
#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#  if defined(WINCE) || defined(_WIN32_WCE)
#    define OPSTR_OS_WINCE
#  else
#    define OPSTR_OS_WIN32
#  endif
#elif defined(__MWERKS__) && defined(__INTEL__)
#  define OPSTR_OS_WIN32
#elif defined(__sun) || defined(sun)
#  define OPSTR_OS_SOLARIS
#elif defined(hpux) || defined(__hpux)
#  define OPSTR_OS_HPUX
#elif defined(__ultrix) || defined(ultrix)
#  define OPSTR_OS_ULTRIX
#elif defined(sinix)
#  define OPSTR_OS_RELIANT
#elif defined(__linux__) || defined(__linux)
#  define OPSTR_OS_LINUX
#elif defined(__FreeBSD__) || defined(__DragonFly__)
#  define OPSTR_OS_FREEBSD
#  define OPSTR_OS_BSD4
#elif defined(__NetBSD__)
#  define OPSTR_OS_NETBSD
#  define OPSTR_OS_BSD4
#elif defined(__OpenBSD__)
#  define OPSTR_OS_OPENBSD
#  define OPSTR_OS_BSD4
#elif defined(__bsdi__)
#  define OPSTR_OS_BSDI
#  define OPSTR_OS_BSD4
#elif defined(__sgi)
#  define OPSTR_OS_IRIX
#elif defined(__osf__)
#  define OPSTR_OS_OSF
#elif defined(_AIX)
#  define OPSTR_OS_AIX
#elif defined(__Lynx__)
#  define OPSTR_OS_LYNX
#elif defined(__GNU__)
#  define OPSTR_OS_HURD
#elif defined(__DGUX__)
#  define OPSTR_OS_DGUX
#elif defined(__QNXNTO__)
#  define OPSTR_OS_QNX6
#elif defined(__QNX__)
#  define OPSTR_OS_QNX
#elif defined(_SEQUENT_)
#  define OPSTR_OS_DYNIX
#elif defined(_SCO_DS) /* SCO OpenServer 5 + GCC */
#  define OPSTR_OS_SCO
#elif defined(__USLC__) /* all SCO platforms + UDK or OUDK */
#  define OPSTR_OS_UNIXWARE
#elif defined(__svr4__) && defined(i386) /* Open UNIX 8 + GCC */
#  define OPSTR_OS_UNIXWARE
#elif defined(__INTEGRITY)
#  define OPSTR_OS_INTEGRITY
#elif defined(__MAKEDEPEND__)
#else
#  error "OS not supported!"
#endif

#if defined(OPSTR_OS_WIN32) || defined(OPSTR_OS_WIN64) || defined(OPSTR_OS_WINCE)
#  define OPSTR_OS_WIN
#endif

#if defined(OPSTR_OS_DARWIN)
#  define OPSTR_OS_MAC /* OPSTR_OS_MAC is mostly for compatibility, but also more clear */
#  define OPSTR_OS_MACX /* OPSTR_OS_MACX is only for compatibility.*/
#  if defined(OPSTR_OS_DARWIN64)
#     define OPSTR_OS_MAC64
#  elif defined(OPSTR_OS_DARWIN32)
#     define OPSTR_OS_MAC32
#  endif
#endif


#if defined(OPSTR_OS_WIN)
	#if defined(__cplusplus)
		#define OPSTR_DllExport extern "C" __declspec( dllexport )
		#define OPSTR_DllImport extern "C" __declspec( dllimport )
	#else
		#define OPSTR_DllExport extern  __declspec( dllexport )
		#define OPSTR_DllImport extern  __declspec( dllimport )
	#endif
	//#define OPSTR_CALL __cdecl
	#define OPSTR_CALL __stdcall
	#define OPSTR_CALLBACK_CALL __stdcall
#else
	#if defined(__cplusplus)
		#define OPSTR_DllExport extern "C" __attribute__ ((visibility("default")))
		#define OPSTR_DllImport extern "C" __attribute__ ((visibility("default")))
	#else
		#define OPSTR_DllExport extern __attribute__ ((visibility("default"))) 
		#define OPSTR_DllImport extern __attribute__ ((visibility("default"))) 
	#endif
	#define OPSTR_CALL
	#define OPSTR_CALLBACK_CALL
#endif

#define OPSTR_API OPSTR_DllExport



// ***************************************************************************
// frequently used integer types
// ***************************************************************************
#ifndef drv_bool
#ifndef __cplusplus
	typedef unsigned char 			bool;
#endif
#endif

#ifndef BOOL
		typedef int                 BOOL;
#endif
#ifndef BYTE
	typedef unsigned char 			BYTE;
#endif
#ifndef WORD
	typedef unsigned short 			WORD;
#endif
#ifndef UINT8
	typedef unsigned char			UINT8;
#endif
#ifndef UINT16
	typedef unsigned short			UINT16;
#endif
#ifndef UINT32
	typedef unsigned int			UINT32;
#endif
#if defined(MSVC) || defined(_MSC_VER)
	#ifndef UINT64
		typedef unsigned __int64	UINT64;
	#endif
	#ifndef INT64
		typedef __int64				INT64;
	#endif
#else
	#ifndef UINT64
		typedef unsigned long long	UINT64;
	#endif
	#ifndef INT64
		typedef long long			INT64;
	#endif
#endif
#ifndef ULONG
typedef unsigned long				ULONG;
#endif

typedef const char*				opstr_LPCTSTR;
typedef UINT8					GvcpFlags;
typedef UINT8					GevIpConfig;

#if defined(_MSC_VER) && (_MSC_VER <= 1200) //MSVC 6.0
	#pragma warning(disable: 4786)
#endif

enum GVCP_FLAGS
{
	GVCP_FLAG_ACK_BIT				= 0x01,	// Ack bit
	GVCP_DISCOVERY_ALLOW_BROADCAST	= 0x10	// allows device to broadcast DISCOVERY_ACK message	
};

// Memory location type
enum opstr_MEM_TYPE
{
	opstr_MEM_CPU = 0,
	opstr_MEM_GPU = 1
};

// Event massage priority
enum opstr_EM_PRIORITY
{
	opstr_EM_PRIORITY_LOW	= 0,
	opstr_EM_PRIORITY_NORMAL	= 1,
	opstr_EM_PRIORITY_HIGH	= 2
};

// Event message type
enum opstr_EM_TYPE
{
	opstr_EM_TYPE_INFO		= 0,
	opstr_EM_TYPE_WARNING	= 1,
	opstr_EM_TYPE_ERROR		= 2
};

// GenICam node types
typedef enum
{
	opstr_NT_UnknowNodeType		= 0,	// Unknown node type. 
	opstr_NT_INode				= 1,	// INode node (all nodes are derived from this type). 
	opstr_NT_ICategory			= 2,	// ICategory node. Represent the Category grouping node type. 
	opstr_NT_IInteger			= 3,	// IInteger node. An 64-bit integer type. 
	opstr_NT_IEnumeration		= 4,	// IEnumeration node. Enumeration type. 
	opstr_NT_IEnumEntry			= 5,	// IEnumEntry node. Enumeration entry type that contains the value of an emumeration. 
	opstr_NT_IMaskedIntReg		= 6,	// IMaskedIntReg node. Masked integer register node. 
	opstr_NT_IRegister			= 7,	// IRegister node. Register node. 
	opstr_NT_IIntReg			= 8,	// IIntReg node. Integer register node. 
	opstr_NT_IFloat				= 9,	// IFloat node. Floating point node. 
	opstr_NT_IFloatReg			= 10,	// IFloatReg node. Floating point register node. 
	opstr_NT_ISwissKnife		= 11,	// ISwissKnife node. Special node used for doing calculations and conversions. 
	opstr_NT_IIntSwissKnife		= 12,	// IIntSwissKnife node. Special node used for doing integer calculations and conversions. 
	opstr_NT_IIntKey			= 13,	// IIntKey node. Integer key node. 
	opstr_NT_ITextDesc			= 14,	// ITextDesc node. Text description node. 
	opstr_NT_IPort				= 15,	// IPort node. Generic port node. 
	opstr_NT_IConfRom			= 16,	// IConfRom node. Configuration node. 
	opstr_NT_IAdvFeatureLock	= 17,	// IAdvFeatureLock node. Advanced feature lock node. 
	opstr_NT_ISmartFeature		= 18,	// ISmartFeature node. Smart feature node. 
	opstr_NT_IStringReg			= 19,	// IStringReg node. String register node. 
	opstr_NT_IBoolean			= 20,	// IBoolean node. Boolean node.
	opstr_NT_ICommand			= 21,	// ICommand node. Command node that can execute a predefined command. 
	opstr_NT_IConverter			= 22,	// IConverter node. Special node used for conversion. 
	opstr_NT_IIntConverter		= 23,	// IIntConverter node. Special node used for integer conversion. 
	opstr_NT_IChunkPort			= 24,	// IChunkPort node. 
	opstr_NT_INodeMap			= 25,	// INodeMap node. 
	opstr_NT_INodeMapDyn		= 26,	// INodeMapDyn node. 
	opstr_NT_IDeviceInfo		= 27,	// IDeviceInfo node. 
	opstr_NT_ISelector			= 28,	// ISelector node. 
	opstr_NT_IPortConstruct		= 29,	// IPortConstruct node.  
	opstr_NT_IString			= 30	// IString node.  
} opstr_NODE_TYPES;

// Access mode of node
typedef enum 
{
	opstr_AM_NI = 0,					// Not implemented
	opstr_AM_NA = 1,					// Not available
	opstr_AM_WO = 2,					// Write Only
	opstr_AM_RO = 3,					// Read Only
	opstr_AM_RW = 4,					// Read and Write
	opstr_AM_UNDEF = 5				// Object is not initialized
} opstr_ACCESS_MODE; 

// Recommended visibility of a node
typedef enum 
{
    opstr_VIS_BEGINNER = 0,			// Always visible
    opstr_VIS_EXPERT = 1,			// Visible for experts or Gurus
    opstr_VIS_GURU = 2,				// Visible for Gurus
    opstr_VIS_INVISIBLE = 3,			// Not Visible
    opstr_VIS_UNDEF  = 99			// Object is not initialized
} opstr_VISIBILITY;

// Caching mode of a register
typedef enum
{
	opstr_CM_NO_CACHE = 0,			// Do not use cache
	opstr_CM_WRITE_THROUGH = 1,		// Write to cache and register
	opstr_CM_WRITE_AROUND = 2,		// Write to register, write to cache on read
	opstr_CM_UNDEF = 3				// Not yet initialized
} opstr_CACHING_MODE;

// Recommended representation of a node value
typedef enum 
{
    opstr_REP_LINEAR = 0,			// Slider with linear behaviour
    opstr_REP_LOGARITHMIC = 1,		// Slider with logarithmic behaviour
    opstr_REP_BOOLEAN = 2,			// Checkbox
    opstr_REP_PURENUMBER = 3,		// Decimal number in an edit control
    opstr_REP_HEX_NUMBER = 4,		// Hex number in an edit control
    opstr_REP_UNDEF = 5				// Not yet initialized
} opstr_REPRESENTATION;

// Endianess of a value in a register
typedef enum 
{
    opstr_END_BIG_ENDIAN = 0,		// Register is big endian
    opstr_END_LITTLE_ENDIAN = 1,		// Register is little endian
    opstr_END_UNDEF = 2				// Object is not yetinitialized
} opstr_ENDIANESS;

// Defines if a node name is standard or custom
typedef enum 
{
    opstr_NS_CUSTOM = 0,				// name resides in custom namespace
    opstr_NS_STANDARD = 1,			// name resides in one of the standard namespaces
    opstr_NS_UNDEF = 2				// Object is not yet initialized
} opstr_NAMESPACE;


// Defines from which standard namespace a node name comes from 
typedef enum 
{
    opstr_SNS_NONE = 0,				// name resides in custom namespace
    opstr_SNS_GEV = 1,				// name resides in GigE Vision namespace
    opstr_SNS_IIDC = 2,				// name resides in 1394 IIDC namespace
    opstr_SNS_CL = 3,				// name resides in camera link namespace
    opstr_SNS_UNDEF = 4				// Object is not yetinitialized
} opstr_STANDARD_NAMESPACE;


//! Defines the chices of a Yes/No alternaitve
//! \ingroup GenApi_PublicUtilities
//typedef enum 
//{
//    Yes,            //!< yes
//    No,                //!< no
//    _UndefinedEYesNo  //!< Object is not yetinitialized
//} EYesNo;

//! typedef for fomula type
//! \ingroup GenApi_PublicImpl
//typedef enum ESlope
//{
//    Increasing, //!> strictly monotonous increasing
//    Decreasing,    //!> strictly monotonous decreasing
//    Varying,    //!> slope changes, e.g. at run-time
//    Automatic,    //!> slope is determined automatically by probing the function
//    _UndefinedESlope  //!< Object is not yetinitialized
//};


// GEV STATUS OF LAST OPERATION

typedef
enum _GEV_STATUS
{
	GEV_STATUS_SUCCESS					= 0x0000,
	GEV_STATUS_NOT_IMPLEMENTED			= 0x8001,
	GEV_STATUS_INVALID_PARAMETER		= 0x8002,
	GEV_STATUS_INVALID_ADDRESS			= 0x8003,
	GEV_STATUS_WRITE_PROTECT			= 0x8004,
	GEV_STATUS_BAD_ALIGNMENT			= 0x8005,
	GEV_STATUS_ACCESS_DENIED			= 0x8006,
	GEV_STATUS_BUSY						= 0x8007,
	GEV_STATUS_LOCAL_PROBLEM			= 0x8008,
	GEV_STATUS_MSG_MISMATCH				= 0x8009,
	GEV_STATUS_INVALID_PROTOCOL			= 0x800A,
	GEV_STATUS_NO_MSG					= 0x800B,
	GEV_STATUS_PACKET_UNAVAILABLE		= 0x800C,
	GEV_STATUS_DATA_OVERRUN				= 0x800D,
	GEV_STATUS_INVALID_HEADER			= 0x800E,

	GEV_STATUS_ERROR					= 0xFFFF
}
	GEV_STATUS;

enum GEV_ENDIANESS
{
	GEV_LITTLE_ENDIAN					= 0x00000000,
	GEV_BIG_ENDIAN 						= 0x80000000
};

enum GEV_CHARACTER_SET
{
	GEV_CH_SET_RESERVED 				= 0x00000000,
	GEV_CH_SET_UTF8 					= 0x00000001
};

enum GEV_IP_CONFIG
{
	GEV_IP_CONFIG_PERSISTENT_IP			= 0x00000001,		// bit 0
	GEV_IP_CONFIG_DHCP					= 0x00000002,		// bit 1
	GEV_IP_CONFIG_LLA					= 0x00000004		// bit 2
};

//===================================================
// PIXEL TYPES
//===================================================
// Indicate if pixel is monochrome or RGB

#define GVSP_PIX_MONO						0x01000000
#define GVSP_PIX_RGB						0x02000000
#define GVSP_PIX_CUSTOM						0x80000000
#define GVSP_PIX_COLOR_MASK					0xFF000000
// Indicate effective number of bits occupied by the pixel (including padding).
// This can be used to compute amount of memory required to store an image.
#define GVSP_PIX_OCCUPY8BIT					0x00080000
#define GVSP_PIX_OCCUPY12BIT				0x000C0000
#define GVSP_PIX_OCCUPY16BIT				0x00100000
#define GVSP_PIX_OCCUPY24BIT				0x00180000
#define GVSP_PIX_OCCUPY32BIT				0x00200000
#define GVSP_PIX_OCCUPY36BIT				0x00240000
#define GVSP_PIX_OCCUPY48BIT				0x00300000
#define GVSP_PIX_EFFECTIVE_PIXEL_SIZE_MASK	0x00FF0000
#define GVSP_PIX_EFFECTIVE_PIXEL_SIZE_SHIFT	16
// Pixel ID: lower 16-bit of the pixel type
#define GVSP_PIX_ID_MASK					0x0000FFFF

typedef
enum _GVSP_PIXEL_TYPES
{
	// Mono
	GVSP_PIX_MONO8				= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY8BIT | 0x0001),
	GVSP_PIX_MONO8_SIGNED		= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY8BIT | 0x0002),
	GVSP_PIX_MONO10				= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0003),
	GVSP_PIX_MONO10_PACKED		= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT | 0x0004),
	GVSP_PIX_MONO12				= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0005),
	GVSP_PIX_MONO12_PACKED		= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT | 0x0006),
	GVSP_PIX_MONO16				= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0007),

	// Bayer
	GVSP_PIX_BAYGR8				= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY8BIT | 0x0008),
	GVSP_PIX_BAYRG8				= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY8BIT | 0x0009),
	GVSP_PIX_BAYGB8				= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY8BIT | 0x000A),
	GVSP_PIX_BAYBG8				= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY8BIT | 0x000B),
	GVSP_PIX_BAYGR10			= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x000C),
	GVSP_PIX_BAYRG10			= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x000D),
	GVSP_PIX_BAYGB10			= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x000E),
	GVSP_PIX_BAYBG10			= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x000F),
	GVSP_PIX_BAYGR12			= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0010),
	GVSP_PIX_BAYRG12			= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0011),
	GVSP_PIX_BAYGB12			= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0012),
	GVSP_PIX_BAYBG12			= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0013),
	GVSP_PIX_BAYGR10_PACKED		= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT  | 0x0026),
	GVSP_PIX_BAYRG10_PACKED		= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT  | 0x0027),
	GVSP_PIX_BAYGB10_PACKED		= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT  | 0x0028),
	GVSP_PIX_BAYBG10_PACKED		= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT  | 0x0029),
	GVSP_PIX_BAYGR12_PACKED		= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT  | 0x002A),
	GVSP_PIX_BAYRG12_PACKED		= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT  | 0x002B),
	GVSP_PIX_BAYGB12_PACKED		= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT  | 0x002C),
	GVSP_PIX_BAYBG12_PACKED		= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY12BIT  | 0x002D),
	GVSP_PIX_BAYGR16			= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x002E),
	GVSP_PIX_BAYRG16			= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x002F),
	GVSP_PIX_BAYGB16			= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0030),
	GVSP_PIX_BAYBG16			= (GVSP_PIX_MONO | GVSP_PIX_OCCUPY16BIT | 0x0031),

	// RGB Packed
	GVSP_PIX_RGB8_PACKED		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY24BIT | 0x0014),
	GVSP_PIX_BGR8_PACKED		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY24BIT | 0x0015),
	GVSP_PIX_RGBA8_PACKED		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY32BIT | 0x0016),
	GVSP_PIX_BGRA8_PACKED		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY32BIT | 0x0017),
	GVSP_PIX_RGB10_PACKED		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY48BIT | 0x0018),
	GVSP_PIX_BGR10_PACKED		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY48BIT | 0x0019),
	GVSP_PIX_RGB12_PACKED		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY48BIT | 0x001A),
	GVSP_PIX_BGR12_PACKED		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY48BIT | 0x001B),
	GVSP_PIX_RGB16				= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY48BIT | 0x0033),
	GVSP_PIX_RGB10V1_PACKED		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY32BIT | 0x001C),
	GVSP_PIX_RGB10V2_PACKED		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY32BIT | 0x001D),

	// YUV
	GVSP_PIX_YUV411_PACKED		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY12BIT | 0x001E),
	GVSP_PIX_YUV422_PACKED		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY16BIT | 0x001F),
	GVSP_PIX_YUV422_YUYV_PACKED	= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY16BIT | 0x0032),
	GVSP_PIX_YUV444_PACKED		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY24BIT | 0x0020),

	// RGB Planar
	GVSP_PIX_RGB8_PLANAR		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY24BIT | 0x0021),
	GVSP_PIX_RGB10_PLANAR		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY48BIT | 0x0022),
	GVSP_PIX_RGB12_PLANAR		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY48BIT | 0x0023),
	GVSP_PIX_RGB16_PLANAR		= (GVSP_PIX_RGB | GVSP_PIX_OCCUPY48BIT | 0x0024)
}
GVSP_PIXEL_TYPES;

#define GVSP_PIX_RGB16_PACKED GVSP_PIX_RGB16; 

typedef
enum _GEV_BOOTSTRAP_REGS_ADDR
{
	// DISCOVERY REGS - START
	VERSION_ADDR						= 0x0000,
	DEVICE_MODE_ADDR					= 0x0004,
	DEVICE_MAC_HIGH0_ADDR				= 0x0008,
	DEVICE_MAC_LOW0_ADDR				= 0x000C,
	SUPPORTED_IP_CONFIG0_ADDR			= 0x0010,
	CURRENT_IP_CONFIG0_ADDR				= 0x0014,
	CURRENT_IP_ADDR0_ADDR				= 0x0024,
	CURRENT_SUBNET0_ADDR				= 0x0034,
	CURRENT_GATEWAY0_ADDR				= 0x0044,
	MANUFACTURER_NAME_ADDR				= 0x0048,
	MODEL_NAME_ADDR						= 0x0068,
	DEVICE_VERSION_ADDR					= 0x0088,
	MANUFACTURER_INFO_ADDR				= 0x00A8,
	SERIAL_NUMBER_ADDR					= 0x00D8,
	USER_DEFINED_NAME_ADDR				= 0x00E8,
	// DISCOVERY REGS - END

	FIRST_URL_ADDR						= 0x0200,
	SECOND_URL_ADDR						= 0x0400,
	
	NUM_NETWORK_INTERFACES_ADDR			= 0x0600,

	PERSISTENT_IP_ADDR0_ADDR			= 0x064C,
	PERSISTENT_SUBNET0_ADDR				= 0x065C,
	PERSISTENT_GATEWAY0_ADDR			= 0x066C,

	DEVICE_MAC_HIGH1_ADDR				= 0x0680,
	DEVICE_MAC_LOW1_ADDR				= 0x0684,
	SUPPORTED_IP_CONFIG1_ADDR			= 0x0688,
	CURRENT_IP_CONFIG1_ADDR				= 0x068C,
	CURRENT_IP_ADDR1_ADDR				= 0x069C,
	CURRENT_SUBNET1_ADDR				= 0x06AC,
	CURRENT_GATEWAY1_ADDR				= 0x06BC,
	PERSISTENT_IP_ADDR1_ADDR			= 0x06CC,
	PERSISTENT_SUBNET1_ADDR				= 0x06DC,
	PERSISTENT_GATEWAY1_ADDR			= 0x06EC,

	DEVICE_MAC_HIGH2_ADDR				= 0x0700,
	DEVICE_MAC_LOW2_ADDR				= 0x0704,
	SUPPORTED_IP_CONFIG2_ADDR			= 0x0708,
	CURRENT_IP_CONFIG2_ADDR				= 0x070C,
	CURRENT_IP_ADDR2_ADDR				= 0x071C,
	CURRENT_SUBNET2_ADDR				= 0x072C,
	CURRENT_GATEWAY2_ADDR				= 0x073C,
	PERSISTENT_IP_ADDR2_ADDR			= 0x074C,
	PERSISTENT_SUBNET2_ADDR				= 0x075C,
	PERSISTENT_GATEWAY2_ADDR			= 0x076C,

	DEVICE_MAC_HIGH3_ADDR				= 0x0780,
	DEVICE_MAC_LOW3_ADDR				= 0x0784,
	SUPPORTED_IP_CONFIG3_ADDR			= 0x0788,
	CURRENT_IP_CONFIG3_ADDR				= 0x078C,
	CURRENT_IP_ADDR3_ADDR				= 0x079C,
	CURRENT_SUBNET3_ADDR				= 0x07AC,
	CURRENT_GATEWAY3_ADDR				= 0x07BC,
	PERSISTENT_IP_ADDR3_ADDR			= 0x07CC,
	PERSISTENT_SUBNET3_ADDR				= 0x07DC,
	PERSISTENT_GATEWAY3_ADDR			= 0x07EC,

	NUM_MSG_CHANNELS_ADDR				= 0x0900,
	NUM_STREAM_CHANNELS_ADDR			= 0x0904,

	GVCP_CAPABILITY_ADDR				= 0x0934,
	HEARTBEAT_TIMEOUT_ADDR				= 0x0938,
	TIMESTAMP_TICK_FREQ_HIGH_ADDR		= 0x093C,
	TIMESTAMP_TICK_FREQ_LOW_ADDR		= 0x0940,
	TIMESTAMP_CTRL_ADDR					= 0x0944,
	TIMESTAMP_VALUE_HIGH_ADDR			= 0x0948,
	TIMESTAMP_VALUE_LOW_ADDR			= 0x094C,

	CCP_ADDR							= 0x0A00,

	MCP_ADDR							= 0x0B00,
	MCDA_ADDR							= 0x0B10,
	MCTT_ADDR							= 0x0B14,
	MCRC_ADDR							= 0x0B18,

	SCP0_ADDR							= 0x0D00,
	SCPS0_ADDR							= 0x0D04,
	SCPD0_ADDR							= 0x0D08,
	SCDA0_ADDR							= 0x0D18
}
GEV_BOOTSTRAP_REGS_ADDR;

// ***************************************************************************
// GVCP defines
// ***************************************************************************

#define GVCP_PORT				3956		// GVCP port
#define GVCP_KEY_VALUE			0x42		// GVCP packet ID
#define GVCP_MAX_DATAGRAM_SIZE	576			// IP + UDP + GVCP + payload
#define GVCP_MAX_PAYLOAD		540			// only GVCP data (without headers)
#define GVCP_MAX_UDP_DATA_SIZE	548			// GVCP header + GVCP payload
#define GVSP_MAX_UDP_DATA_SIZE	65535		// GVSP header + GVSP payload

#define GVCP_INADDR_BROADCAST	0xFFFFFFFF 	// IP broadcast address

#define GVCP_BOOTSTRAP_REG_SIZE	0xA000		// size of GigE bootstrap register space

#define GVCP_READREG_MAX_COUNT	135 * 4		// 540 - max. bytes READREG command can retreive
#define GVCP_WRITEREG_MAX_COUNT	67 * 8		// 536 - max. bytes WRITEREG command can send
#define GVCP_READMEM_MAX_COUNT	GVCP_MAX_PAYLOAD - 4 // max. bytes READMEM command can retreive
#define GVCP_WRITEMEM_MAX_COUNT	GVCP_READMEM_MAX_COUNT



#ifndef __cplusplus
	#if defined (OPSTR_OS_WIN)
		#define inline __inline
	#else
		#define inline __inline__
	#endif
#endif // __cplusplus



static inline UINT32 IsMonoPixelType(UINT32 pixelType)
{
	return ((GVSP_PIX_ID_MASK & pixelType) >= (GVSP_PIX_ID_MASK & GVSP_PIX_MONO8) &&
			(GVSP_PIX_ID_MASK & pixelType) <= (GVSP_PIX_ID_MASK & GVSP_PIX_MONO16));
}



static inline UINT32 GvspGetBitsPerPixel(GVSP_PIXEL_TYPES pixelType)
{
	return ((pixelType & GVSP_PIX_EFFECTIVE_PIXEL_SIZE_MASK) >>	GVSP_PIX_EFFECTIVE_PIXEL_SIZE_SHIFT);
}



static inline UINT8 GvspGetBitsDepth(GVSP_PIXEL_TYPES pixelType)
{
	switch (pixelType) {
	case GVSP_PIX_MONO8:
	case GVSP_PIX_MONO8_SIGNED:
	case GVSP_PIX_BAYGR8:
	case GVSP_PIX_BAYRG8:
	case GVSP_PIX_BAYGB8:
	case GVSP_PIX_BAYBG8:
	case GVSP_PIX_RGB8_PACKED:
	case GVSP_PIX_BGR8_PACKED:
	case GVSP_PIX_RGBA8_PACKED:
	case GVSP_PIX_BGRA8_PACKED:
	case GVSP_PIX_RGB8_PLANAR:
		return 8;
	case GVSP_PIX_MONO10:
	case GVSP_PIX_MONO10_PACKED:
	case GVSP_PIX_BAYGR10:
	case GVSP_PIX_BAYRG10:
	case GVSP_PIX_BAYGB10:
	case GVSP_PIX_BAYBG10:
	case GVSP_PIX_BAYGR10_PACKED:
	case GVSP_PIX_BAYRG10_PACKED:
	case GVSP_PIX_BAYGB10_PACKED:
	case GVSP_PIX_BAYBG10_PACKED:
	case GVSP_PIX_RGB10_PACKED:
	case GVSP_PIX_BGR10_PACKED:
	case GVSP_PIX_RGB10V1_PACKED:
	case GVSP_PIX_RGB10V2_PACKED:
	case GVSP_PIX_RGB10_PLANAR:
		return 10;
	case GVSP_PIX_MONO12:
	case GVSP_PIX_MONO12_PACKED:
	case GVSP_PIX_BAYGR12:
	case GVSP_PIX_BAYRG12:
	case GVSP_PIX_BAYGB12:
	case GVSP_PIX_BAYBG12:
	case GVSP_PIX_BAYGR12_PACKED:
	case GVSP_PIX_BAYRG12_PACKED:
	case GVSP_PIX_BAYGB12_PACKED:
	case GVSP_PIX_BAYBG12_PACKED:
	case GVSP_PIX_RGB12_PACKED:
	case GVSP_PIX_BGR12_PACKED:
	case GVSP_PIX_RGB12_PLANAR:
		return 12;
	default:
		return 16;
	}
}



static inline UINT32 GvspGetRawDataSize(UINT32 sizeX, UINT32 sizeY, UINT32 paddingX, UINT32 paddingY,
								 GVSP_PIXEL_TYPES pixelType)
{
	return (((sizeX * sizeY * GvspGetBitsPerPixel(pixelType) + 7) / 8) + (sizeY * paddingX) + (paddingY));
}



static inline UINT32 GvspGetLineSizeBits(UINT32 sizeX, UINT32 paddingX, GVSP_PIXEL_TYPES pixelType)
{
		return (sizeX * GvspGetBitsPerPixel(pixelType) + (paddingX * 8));
}



static inline UINT32 GvspGetLineSizeBytes(UINT32 sizeX, UINT32 paddingX, GVSP_PIXEL_TYPES pixelType)
{
		return ((GvspGetLineSizeBits(sizeX, paddingX, pixelType) + 7) / 8);
}



static inline UINT16 GevSwapWord(UINT16 value)
{
	return (((value & 0xFF00) >> 8) |
			((value & 0x00FF) << 8));
}



static inline UINT32 GevSwapDWord(UINT32 value)
{
	return (((value & 0xFF000000) >> 24) |
			((value & 0x00FF0000) >> 8) |
			((value & 0x0000FF00) << 8) |
			((value & 0x000000FF) << 24));
}

/**	Packets Statistics.
 */
typedef
struct _PacketStatistics
{
	UINT64 packetsReceivedTotal;
	UINT64 packetsReceivedHeaderError;
	UINT64 packetsReceivedLeader;
	UINT64 packetsReceivedPayload;
	UINT64 packetsReceivedTrailer;
	UINT64 packetsDroppedLeader;
	UINT64 packetsDroppedPayload;
	UINT64 packetsDroppedTrailer;
	UINT64 LeaderOverLeader;
	
	UINT64 packetsAllDevicesTotal;
	UINT64 packetsUnknownDeviceTotal;
	UINT64 packetsAllDevicesLeader;
	UINT64 packetsAllDevicesPayload;
	UINT64 packetsAllDevicesTrailer;
	UINT64 packetsAllDevicesUnknown;

	// cause of leader packet drop
	UINT64 leaderDropBufferFull;
	UINT64 leaderDropImageInfoNull;
	UINT64 leaderDropAllocatePoolError;

	//TODO TEST
	UINT64 packetsAllDevicesTotal_PtReceive;	//TODO TEST
	UINT64 packetsAllDevicesTotal_PtReceivePkt;	//TODO TEST
	UINT64 packetsDroppedPayloadBlockID;
	UINT64 packetsDroppedPayloadWrongType;
	UINT64 packetsDroppedPayloadImgBufFull;

}
	PacketStatistics, *PPacketStatistics;

/**	Image info from engine.
 */
typedef
struct _GigEImageInfo
{
	/**	ID index number.
	 */
	UINT32 ID;
	/**	Block ID (frame ID from camera).
	 */
	UINT32 BlockID;
	/**	Pointer to raw data buffer.
	 */
	UINT64 RawData;
	/**	Image pixel type.
	 */
	UINT32 PixelType;
	/**	Size of raw data buffer.
	 */
	UINT32 RawDataSize;
	/**	Size of the line in bytes. Including padding.
	 */
	UINT32 LineSize;	
	/**	Image timestamp.
	 */
	UINT64 Timestamp;
	/**	Camera image timestamp.
	 */
	UINT64 CameraTimestamp;
	/** Image size.
	 */
	UINT32 SizeX;
	UINT32 SizeY;
	/**	Image offset.
	 */
	UINT32 OffsetX;
	UINT32 OffsetY;
	/**	Image padding.
	 */
	UINT32 PaddingX;
	UINT32 PaddingY;

	/** Packets Statistics
	 */
	PacketStatistics packetStats;
	BOOL isOnHold;
	BOOL isFull;

	UINT16 payloadType;
//
//	UINT32 NumberOfPacketsInBlock;
//	UINT32 PacketsReceivedForBlock;
//	UINT8  PacketsReceived[1500];
//	UINT32 PacketSize;
//	UINT8  FrameErrorFree;
}
	GigEImageInfo, *PGigEImageInfo;



enum API_PARAMETERS_STATISTICS
{
	// API parameters
	ApiMaxImageSize							= 0xA001,

	ApiEnablePacketResend					= 0xB001,
	ApiAcceptIncompleteImage				= 0xB002,
	ApiPacketResendResponseTimeout			= 0xB004,
	ApiMaxResendPacketRetry					= 0xB005,
	ApiMaxMissingPacketWaiting				= 0xB006,
	ApiMaxNextPacketWaiting					= 0xB007,
	ApiMaxMissingPacketsCount				= 0xB008,
	ApiMaxNewImagesPending					= 0xB009,
	ApiMaxNewPacketsPending					= 0xB00A,
	ApiMaxIncompletePackets					= 0xB00B,
	
	// API statistics
	ApiMissingPackets						= 0xB101,
	ApiPacketResendsAmount					= 0xB102,
	ApiLostPackets							= 0xB103,
	ApiLostImages							= 0xB104,
	ApiIgnoredPackets						= 0xB105,
	ApiIncompleteImages						= 0xB106,

	ApiAllPackets							= 0xB181,
	ApiUnknownDevice						= 0xB182,
	ApiLeaderPackets						= 0xB183,
	ApiPayloadPackets						= 0xB184,
	ApiTrailerPackets						= 0xB185,
	ApiUnknownPackets						= 0xB186,
	
	ImageBufferFrameCount					= 0xD001,

	XmlControlLocation						= 0xD100,
	XmlControlFile							= 0xD101,

	// Reset all statistics in API
	ApiResetAll								= 0xB666,
	// Set parameters to default values in API
	ApiSetParametersToDefault				= 0xB777
};

typedef
enum _XML_LOCATION
{	
	DEFAULT_LOCATION						= 0,
	FILE_LOCATION							= 1
} XML_LOCATION;



enum DEVICE_PARAMETERS_STATISTICS
{
	// Device statistics
	DeviceMissingPackets					= 0xC001,
	DevicePacketResendsAmount				= 0xC002,
	DeviceLostPackets						= 0xC003,
	DeviceLostImages						= 0xC004,
	DeviceIgnoredPackets					= 0xC005,
	DeviceIncompleteImages					= 0xC006,

	// Reset all statistics for device
	DeviceResetAll							= 0xC666
};



enum IMAGE_PROC_ALGORITHMS_CLASS
{
	opstr_IPLC_WHITE_BALANCE					= 1,
	opstr_IPLC_AUTO_EXPOSURE					= 2,
	opstr_IPLC_SHARPEN						= 3,
	opstr_IPLC_DEMOSAIC						= 4,
	opstr_IPLC_OTHER							= 5
};



enum IMAGE_PROC_BORDER_TYPE
{
	opstr_IPBT_BILINEAR_BORDER				= 0,
	opstr_IPBT_BLACK_BORDER					= 1,
	opstr_IPBT_CROP_BORDER					= 2,
	opstr_IPBT_COLORIZED_BORDER				= 3,
	opstr_IPBT_COPY_BORDER					= 4
};



typedef UINT32 PixBGRA;



static inline UINT8 PixAlphaBGRA(PixBGRA rgb)										// get alpha part of RGBA
{	
	return (UINT8) (rgb >> 24);
}



static inline UINT8 PixRedBGRA(PixBGRA rgb)											// get red part of RGBA
{
	return (UINT8) (rgb >> 16);
}



static inline UINT8 PixGreenBGRA(PixBGRA rgb)										// get green part of RGBA
{
	return (UINT8) (rgb >> 8);
}



static inline UINT8 PixBlueBGRA(PixBGRA rgb)										// get blue part of RGBA
{
	return (UINT8) (rgb);
}



static inline PixBGRA SetPixBGR32(UINT8 r, UINT8 g, UINT8 b)						// set RGB32 value
{ 
	return (0xFF000000) | (r << 16) | (g << 8) | b; 
}



static inline PixBGRA SetPixBGRA(UINT8 a, UINT8 r, UINT8 g, UINT8 b)				// set RGBA value
{ 
	return (a << 24) | (r << 16) | (g << 8) | b; 
}

/**	Device API init.
 */
typedef
struct _DeviceInit
{
	INT64 sensorWidth;
	INT64 sensorHeight;
	INT64 packetSize;
}
	DeviceInit, *PDeviceInit;


typedef 
struct _SingleLinkedList
{
	struct _SingleLinkedList* next;
	UINT32 id;
}
	SingleLinkedList, *PSingleLinkedList;



#ifdef __cplusplus


#include <vector>
#include <string>
#include <time.h>

namespace opstr {

/**	Event message.
 */
typedef
struct _EventMessage
{
	/**	Message priority.
	 */
	opstr_EM_PRIORITY priority;
	/**	Message timestamp.
	 */
	time_t timestamp;
	/**	Message Type.
	 */
	opstr_EM_TYPE messageType;
	/**	Message code.
	 */
	UINT32 messageCode;
	/**	Message string.
	 */
	std::string messageString;
}
	EventMessage, *PEventMessage;

typedef std::vector<EventMessage> EventMessageVector;

} // namespace opstr

#endif



#endif // INCLUDED_opstr_c_GigEDataTypes_h