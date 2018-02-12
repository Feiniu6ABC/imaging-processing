/***********************************************************************************************/
/***********************************************************************************************/
//////// This program: 1) ask user to enter a color bmp file (24 bits RGB)    ///////////////////
///////                2) reads the bmp file                                  //////////////////
///////                3) convert to greyscale (1 channel 8 bits/pixel         /////////////////
//////                 4) save to an output file                                ////////////////

/***********************************************************************************************/
/***********************************************************************************************/

#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include <cstdlib>
#include <string.h>
#include <bmpImage.h>
#include <iomanip> 

using namespace std;


int main () 
{
  int i, j, length, R, G, B, avg, grey_pad, grey_size_per_row, avg_width, size_per_row, temp_size, index, k;
  string in_file, path, in_file_loc, out_file_loc, file_ext="_out.bmp";

  char out_file[64]="";
  bool extra_b = false;
  unsigned char extraBuff_v[1024], *extra_ptr;
  
  
  while(true)
  {
		in_file_loc.clear();
		out_file_loc.clear();
		in_file.clear();
		
		memset(out_file, 0, sizeof(out_file));		
		
  		cout <<"Please enter a Directory Name, or Enter 'q' to exit the program: ";
  		cin>>path;
  		
  		if(path == "q")
  			break;		
  		
  		cout <<"Please enter a bmp file name(example: 'filename.bmp'): ";
    	cin >> in_file;
    
    	in_file_loc.append(path);
    	in_file_loc.append("\\");
    	in_file_loc.append(in_file);
   
   		FILE* f = fopen(in_file_loc.c_str(), "rb");
    
    	unsigned char info[54];
    	FILE_HEADER file_header, *fh_ptr;
    
    	if(f)
    	{
    		cout <<"Success opening '" <<in_file << "' file" <<endl;  
			fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
		
		//	cout << info[0] <<endl;
		//	cout << info[1] <<endl;
			int bmp_size = *(int*)&info[2];
			cout <<"bmp file size: " <<bmp_size<<endl;

	    	int offset=*(int*)&info[10];
			int DIB_size= *(int*)&info[14];
		
	    	int width= *(int*)&info[18];
	    	int height = *(int*)&info[22];
		
			short bit_planes= *(short*)&info[26];
		
			short bit_counts= *(short*)&info[28];
	
			int bit_compress= *(int*)&info[30];
		
			int image_size = *(int*)&info[34];
		
			int pix_resln_horiz= *(int*)&info[38];
		
			int pix_resln_verti = *(int*)&info[42];
		
			int color_used = *(int*)&info[46];

			int import_color = *(int*)&info[50];

			//find padded bytes per row
			int padd_bytes = 4-( (width*3)%4 );
		
			if(padd_bytes < 4)
			{
				size_per_row = (3*width + padd_bytes);
			}
			else
			{
				size_per_row = (3*width);
				padd_bytes = 0;
			}
		
			unsigned char* padBuf = new unsigned char [padd_bytes*height];  //height x bytes per row
			memset(padBuf, 0, sizeof(padBuf) );
		
			//allocate memory space for each row 1-D 
			unsigned char* data_image = new unsigned char[3*width];
		
			//allocate memory space for avg data(output) 2-D
			unsigned char* avg_data = new unsigned char[width*height];
		
		
			// if NOT 54 header bytes, read extra bytes before reading the image data
			int extra_bytes =0;
			if (offset != 54)
			{
				memset(extraBuff_v, 0, sizeof(extraBuff_v) );
				extra_bytes=offset-54;
				fread(extraBuff_v, sizeof(unsigned char), extra_bytes, f);
				extra_b = true;
			}		
		
			k=0;
			// starting exacting image data, row by row
			for(i=0; i<height; i++)
			{
				fread(data_image, sizeof(unsigned char), 3*width, f);  //read input data one row at a time
		    
				for(j=0; j<3*width; j+=3)  //each column
				{
				
					B = data_image[j];  //read R, G, B value (3 bytes)
					G = data_image[j+1];
					R = data_image[j+2];
				//	avg = (B+G+R)/3;
					avg =  (0.2126*B)+(0.7152*G)+(0.0722*R) ;
				
					avg_data[k] = avg;	
				
					k++;
				
				}
			
			//if theres padding bytes, read 
				if(padd_bytes > 0) 
				{
					fread(padBuf, sizeof(unsigned char), padd_bytes, f); 
				
				} //end if read padd_bytes
			
			}	//end for loop;
		
			//--------------------------------------------------------------------------------------
			//update output file header (8 bit grayscale)
		 
			*(short*)&info[28] = 8;
		
	    	bit_counts= *(short*)&info[28];
		
			bit_compress= *(int*)&info[30];
		
        	//image size new 8 bits per pixel
			*(int*)&info[34] = (padd_bytes + width) * height;
		
			image_size = *(int*)&info[34];
		
			*(int*)&info[2] = offset + image_size;
		
			bmp_size = *(int*)&info[2];
		
			*(int*)&info[46] = 256;
			color_used = *(int*)&info[46];
		
			*(int*)&info[10] = color_used*4+54;
			offset = *(int*)&info[10];
		
    		//--------------------------------------------------------------------------------------------	
    		//add extra offset bytes
    		memset(extraBuff_v, 0, sizeof(extraBuff_v) );
    	
    		extra_ptr=&extraBuff_v[0];
    		unsigned int colorV=0;
    	
    		for (int a=0; a<256; a++)
    		{
    			for (int b=0; b<3; b++)
    			{
    				*extra_ptr = (unsigned char)colorV; 
    				extra_ptr++;
				}
				extra_ptr++;
				colorV++;
			}
    
    	
    		extra_bytes=offset-54;
			extra_b = true;
    	
    		//====================================================================================================
			size_t write_bytes=0, write_padd=0, write_total=0, write_old=0, write_head = 0, str1, extra_write=0;
	    
	    	str1 = strcspn(in_file.c_str(), "." );
	   
	    	strncpy(out_file, in_file.c_str(), str1);
	    	strncat(out_file, file_ext.c_str(), sizeof(file_ext) );
	   
	    	out_file_loc.append(path);
	    	out_file_loc.append("\\");
	    	out_file_loc.append(out_file);
	    	cout <<"output file complete location: " <<out_file_loc<<endl;
	  
	    	FILE* fout = fopen(out_file_loc.c_str(), "wb");
	  
	    	//write start here .....
	  
			write_head = fwrite(info, sizeof(unsigned char), 54, fout);  //output updated header
		
			//if theres extra bytes between header and image data, write to the output file
			if ( extra_b == true)
			{
				extra_write=fwrite(extraBuff_v, sizeof(unsigned char), extra_bytes, fout); 
			}
		
		
			//write output data
			for(i=0; i<height; i++)
			{

		  		write_bytes = fwrite(avg_data+(width*i), sizeof(unsigned char), width, fout);
		  		write_padd = fwrite(padBuf+(i*padd_bytes), sizeof(unsigned char), padd_bytes, fout) ; //pad values=original data
		 
		  
		  		write_old = write_bytes+write_padd;
		  		write_total = write_total + write_old;
		  
			}
 
        	cout <<"write file total bytes: " <<write_total<<endl;
		
			delete[] data_image;
			delete[] avg_data;
	
			fclose(f); //close input file
			fclose(fout);  //close output file
		}
		else
		{
			cout <<"Can't open the file!";
		
		}	
		
		cout <<endl;
		cout <<endl;
		
	}  //end of while
	 	
	 return 0;

 
}
