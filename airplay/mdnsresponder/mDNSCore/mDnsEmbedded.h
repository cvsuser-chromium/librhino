/*                                                                                                                                                                                        
 *      Copyright (C) 2005-2013 Team XBMC                                                                                                                                                 
 *      http://xbmc.org                                                                                                                                                               
 *                                                                                                                                                                                        
 *  This Program is free software; you can redistribute it and/or modify                                                                                                                  
 *  it under the terms of the GNU General Public License as published by                                                                                                                  
 *  the Free Software Foundation; either version 2, or (at your option)                                                                                                                   
 *  any later version.                                                                                                                                                                    
 *                                                                                                                                                                                        
 *  This Program is distributed in the hope that it will be useful,                                                                                                                       
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                                                                                                                        
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                                                                                                                          
 *  GNU General Public License for more details.                                                                                                                                          
 *                                                                                                                                                                                        
 *  You should have received a copy of the GNU General Public License                                                                                                                     
 *  along with XBMC; see the file COPYING.  If not, see                                                                                                                                   
 *  <http://www.gnu.org/licenses/>.                                                                                                                                                       
 *                                                                                                                                                                                        
 */

#ifndef _MDNSEMBEDDED_H_
#define _MDNSEMBEDDED_H_
#ifdef __cplusplus
extern "C" {
#endif
int embedded_mDNSInit();
void embedded_mDNSExit();
int embedded_mDNSmainLoop(struct timeval timeout);
#ifdef __cplusplus
}
#endif
#endif //_MDNSEMBEDDED_H_
