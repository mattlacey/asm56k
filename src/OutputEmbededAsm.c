/*

Project:    asm56k
Author:     M.Buras (sqward)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <asm_types.h>
#include <export.h>
#include <Value.h>
#include "SymbolTable.h"
#include "CodeUtils.h"
#include "OutputLod.h"


/*
            offset and skip are used for L memory only
*/

void SaveDataEmbeded(FILE* output_file,int chunkIndex, int MemType, int offset, int skip)
{
    int j;
    int mod_cnt;
    unsigned char* code;

    mod_cnt = 0;
    code = chunks[chunkIndex].code_ptr + offset;

    debugprint("codelen %d, %d\r\n",chunks[chunkIndex].code_len/3,chunks[chunkIndex].code_len);

    j = (chunks[chunkIndex].code_len/3) >> ( skip ? 1 : 0 ) ;

    fprintf(output_file,"\n  dc.b $00,$00,$%.2x\n",MemType);
    fprintf(output_file,"  dc.b $%.2x,$%.2x,$%.2x\n",(chunks[chunkIndex].pc>>16)&0xff,(chunks[chunkIndex].pc>>8)&0xff,(chunks[chunkIndex].pc)&0xff);
    fprintf(output_file,"  dc.b $%.2x,$%.2x,$%.2x\n",(j>>16)&0xff,(j>>8)&0xff,(j)&0xff);

    if ( j != 0 )
        fprintf(output_file,"  dc.b ");

    for( ; j != 0; j-- )
    {

        if(mod_cnt==7 || j==1)
        {
            fprintf(output_file,"$%.2x,$%.2x,$%.2x\n",code[0],code[1],code[2]);
            if ( j > 1 )
            {
                fprintf(output_file,"  dc.b ");
            }
        }
        else
        {
            fprintf(output_file,"$%.2x,$%.2x,$%.2x,",code[0],code[1],code[2]);
        }

        code += ( 3 + skip );

        mod_cnt++;
        mod_cnt&=0x7;
    }
}


void SaveFileEmbeded( const char* name )
{
	FILE *output_file;
	int i;
    char baseName[256];
    char* pBaseName = NULL;
    char* pFileExt = NULL;

	output_file=fopen(name,"wb");
	if(output_file<=0)
	{
		printf("error while opening file: %s for write.\n",name);
		return;
	}
    
    strcpy ( baseName,name);
    pBaseName = strrchr ( baseName,'/' );

    if ( pBaseName == NULL )
    {
        pBaseName = strrchr ( baseName,'\\' );
        if ( pBaseName == NULL )
        {   /* huh, something went wrong. */
            pBaseName = baseName;
        }
    }

    if ( pBaseName[0] != 0 && pBaseName[1] != 0 )
    {
        pBaseName ++;
    }

    pFileExt = strrchr ( pBaseName, '.' );
    
    if ( pFileExt )
    {
        pFileExt[0] = 0;
    }

	fprintf(output_file,"  xdef _EmbededP56_%s\n\n",pBaseName);
	fprintf(output_file,"  xdef _EmbededP56_%s_size\n\n",pBaseName);

	if(num_chunks==0)
	{
		return;
	}

    fprintf(output_file,"_EmbededP56_%s:\n",pBaseName);

	for( i = 0; i != num_chunks2 ;i++ )
	{	
		int j;
        int num_zeros = 0;

        for ( j = 0; j < chunks[i].code_len; j++ )
        {
            if ( chunks[i].code_ptr[j] == 0 )
            {
                num_zeros ++;
            }
        }

        if ( num_zeros != chunks[i].code_len || g_write_zero_sections ) 
        {
		    switch(chunks[i].mem_type)
		    {
			    case P_MEM:  SaveDataEmbeded(output_file, i, chunks[i].mem_type, 0, 0 ); break;
			    case X_MEM:  SaveDataEmbeded(output_file, i, chunks[i].mem_type, 0, 0 ); break;
			    case Y_MEM:  SaveDataEmbeded(output_file, i, chunks[i].mem_type, 0, 0 ); break;
                case L_MEM:  SaveDataEmbeded(output_file, i, 'X', 3, 3 );
                                    SaveDataEmbeded(output_file, i, 'Y', 0, 3 ); break;
            }
        }
	}
	
    fprintf(output_file,"_EmbededP56_%s_size:  dc.l (_EmbededP56_%s_size-_EmbededP56_%s)/3\n",pBaseName,pBaseName,pBaseName);

	fprintf(output_file,"  end\n");

    fclose( output_file );
}
