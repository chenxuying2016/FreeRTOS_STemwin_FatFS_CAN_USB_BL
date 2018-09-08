/*
*********************************************************************************************************
*
*	模块名称 : FatFS接口文件
*	文件名称 : diskio.c
*	版    本 : V1.0
*	说    明 : 支持SD卡和U盘
*
*	修改记录 :
*		版本号   日期         作者           说明
*       v1.0    2014-06-19   Eric2013        首发
*
*	Copyright (C), 2014-2015, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "diskio.h"		    /* FatFs lower layer API */
#include "bsp.h"
#include "usbh_bsp_msc.h"	/* 提供U盘的读写函数 */


#define SECTOR_SIZE		512
/*-------------------------------------------------------------------------------------------*/
/* Inidialize a Drive                                                                        */
/*-------------------------------------------------------------------------------------------*/
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat = STA_NOINIT;

	switch (pdrv)
	{
		case FS_SD :		/* SD卡 */
			if (SD_Init() == SD_OK)
			{
				stat = RES_OK;
			}
			else
			{
				stat = STA_NODISK;
			}
			break;
			
		case FS_NAND :		/* NAND Flash */
//			if (NAND_Init() == NAND_OK)
//			{
//				stat = RES_OK;
//			}
//			else
//			{
//				/* 如果初始化失败，请执行低级格式化 */
//				printf("NAND_Init() Error!  \r\n");
//				stat = RES_ERROR;
//			}
			break;
		
		case FS_USB :		/* STM32 USB Host 口外接U盘 */
			if(HCD_IsDeviceConnected(&USB_OTG_Core))
			{
				stat &= ~STA_NOINIT;
			}
			break;

		default :
			break;
	}

	return stat;
}

/*-------------------------------------------------------------------------------------------*/
/* Get Disk Status                                                                           */
/*-------------------------------------------------------------------------------------------*/
DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat = STA_NOINIT;

	switch (pdrv)
	{
		case FS_SD :
			stat = 0;
			break;
		
		case FS_NAND :
			stat = 0;
			break;

		case FS_USB :
			stat = 0;
			break;

		default:
			stat = 0;
			break;
	}
	return stat;
}

/*-------------------------------------------------------------------------------------------*/
/* Read Sector(s)                                                                            */
/*-------------------------------------------------------------------------------------------*/
DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
)
{
	DRESULT res = RES_OK;

	switch (pdrv)
	{
		case FS_SD :
		{
			SD_Error Status = SD_OK;

			if (count == 1)
			{
				Status = SD_ReadBlock(buff, sector << 9 , SECTOR_SIZE);
			}
			else
			{
				Status = SD_ReadMultiBlocks(buff, sector << 9 , SECTOR_SIZE, count);
			}
			
			if (Status != SD_OK)
			{
				res = RES_ERROR;
				break;
			}

		#ifdef SD_DMA_MODE
			/* SDIO工作在DMA模式，需要检查操作DMA传输是否完成 */
			Status = SD_WaitReadOperation();
			if (Status != SD_OK)
			{
				res = RES_ERROR;
				break;
			}

			while(SD_GetStatus() != SD_TRANSFER_OK);
		#endif

			res = RES_OK;
			break;
		}
		
		case FS_NAND :
//			if (NAND_OK == NAND_ReadMultiSectors(buff, sector, 512, count))
//			{
//				res = RES_OK;
//			}
//			else
//			{
//				printf("NAND_ReadMultiSectors() Error! sector = %d, count = %d \r\n", sector, count);
//				res = RES_ERROR;
//			}
			break;

		case FS_USB :
			//res = USB_disk_read(buff, sector, count);
			{
				BYTE status = USBH_MSC_OK;

				//if (Stat & STA_NOINIT) 	return RES_NOTRDY;

				if (HCD_IsDeviceConnected(&USB_OTG_Core))
				{
					do
					{
						status = USBH_MSC_Read10(&USB_OTG_Core, buff,sector,512 * count);
						USBH_MSC_HandleBOTXfer(&USB_OTG_Core ,&USB_Host);

						if (!HCD_IsDeviceConnected(&USB_OTG_Core))
						{
							break;
						}
					}
					while (status == USBH_MSC_BUSY );
				}

				if (status == USBH_MSC_OK)
				{
					res = RES_OK;
				}
				else
				{
					res = RES_ERROR;
				}
			}
			break;
			
		default:
			res = RES_PARERR;
			break;
	}
	
	return res;
}

/*-------------------------------------------------------------------------------------------*/
/* Write Sector(s)                                                                           */
/*-------------------------------------------------------------------------------------------*/
#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
	DRESULT res = RES_OK;

	switch (pdrv)
	{
		case FS_SD :
		{
			SD_Error Status = SD_OK;

			if (count == 1)
			{
				Status = SD_WriteBlock((uint8_t *)buff, sector << 9 ,SECTOR_SIZE);
			}
			else
			{
				/* 此处存在疑问： 扇区个数如果写 count ，将导致最后1个block无法写入 */
				//Status = SD_WriteMultiBlocks((uint8_t *)buff, sector << 9 ,SECTOR_SIZE, count);
				Status = SD_WriteMultiBlocks((uint8_t *)buff, sector << 9 ,SECTOR_SIZE, count + 1);
			}
			
			if (Status != SD_OK)
			{
				res = RES_ERROR;
				break;
			}

		#ifdef SD_DMA_MODE
			/* SDIO工作在DMA模式，需要检查操作DMA传输是否完成 */
			Status = SD_WaitReadOperation();
			if (Status != SD_OK)
			{
				res = RES_ERROR;
				break;
			}
			while(SD_GetStatus() != SD_TRANSFER_OK);
		#endif
			
			res = RES_OK;
			break;
		}
		
		case FS_NAND :
//			if (NAND_OK == NAND_WriteMultiSectors((uint8_t *)buff, sector, 512, count))
//			{
//				res = RES_OK;
//			}
//			else
//			{
//				printf("NAND_ReadMultiSectors() Error! sector = %d, count = %d \r\n", sector, count);
//				res = RES_ERROR;
//			}
			break;

		case FS_USB :
			//res = USB_disk_write(buff, sector, count);
			{
				BYTE status = USBH_MSC_OK;

				//if (drv || !count) return RES_PARERR;

				//if (Stat & STA_NOINIT) return RES_NOTRDY;
				//if (Stat & STA_PROTECT) return RES_WRPRT;

				if (HCD_IsDeviceConnected(&USB_OTG_Core))
				{
					do
					{
						status = USBH_MSC_Write10(&USB_OTG_Core,(BYTE*)buff,sector, 512 * count);
						USBH_MSC_HandleBOTXfer(&USB_OTG_Core, &USB_Host);

						if(!HCD_IsDeviceConnected(&USB_OTG_Core))
						{
							break;
						}
					}
					while(status == USBH_MSC_BUSY );

				}

				if (status == USBH_MSC_OK)
				{
					res = RES_OK;
				}
				else
				{
					res = RES_ERROR;
				}
			}
			break;

		default:
			res = RES_PARERR;
			break;
	}
	return res;
}
#endif


/*-------------------------------------------------------------------------------------------*/
/* Miscellaneous Functions                                                                   */
/*-------------------------------------------------------------------------------------------*/
#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_PARERR;

	switch (pdrv) {
	case FS_SD :
		res = RES_ERROR;
		switch (cmd)
		{
			/* SD卡磁盘容量： SDCardInfo.CardCapacity */
			case CTRL_SYNC :		/* Wait for end of internal write process of the drive */
				res = RES_OK;
				break;

			case GET_SECTOR_COUNT :	/* Get drive capacity in unit of sector (DWORD) */
				*(DWORD*)buff = SDCardInfo.CardCapacity / 512;
				res = RES_OK;
				break;

			case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
				*(WORD*)buff = 512;
				res = RES_OK;
				break;

			case CTRL_ERASE_SECTOR: /* Erase a block of sectors (used when _USE_ERASE == 1) */
			default:
				res = RES_PARERR;
				break;
		}
		break;

	case FS_NAND :
		res = RES_OK;
		break;

	case FS_USB :
		//if (drv) return RES_PARERR;
		//if (Stat & STA_NOINIT) return RES_NOTRDY;
		switch (cmd)
		{
			case CTRL_SYNC :		/* Make sure that no pending write process */
				res = RES_OK;
				break;

			case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
				*(DWORD*)buff = (DWORD) USBH_MSC_Param.MSCapacity;
				res = RES_OK;
				break;

			case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
				*(WORD*)buff = 512;
				res = RES_OK;
				break;

			case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */\
				*(DWORD*)buff = 512;
				res = RES_OK;
				break;

			default:
				res = RES_PARERR;
				break;
		}
		break;

	default:
		res = RES_PARERR;
		break;
	}
	return res;
}
#endif

/*
*********************************************************************************************************
*	函 数 名: get_fattime
*	功能说明: 获得系统时间，用于改写文件的创建和修改时间。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
DWORD get_fattime (void)
{
	/* 如果有全局时钟，可按下面的格式进行时钟转换. 这个例子是2013-01-01 00:00:00 */

	return	  ((DWORD)(2013 - 1980) << 25)	/* Year = 2013 */
			| ((DWORD)1 << 21)				/* Month = 1 */
			| ((DWORD)1 << 16)				/* Day_m = 1*/
			| ((DWORD)0 << 11)				/* Hour = 0 */
			| ((DWORD)0 << 5)				/* Min = 0 */
			| ((DWORD)0 >> 1);				/* Sec = 0 */
}
