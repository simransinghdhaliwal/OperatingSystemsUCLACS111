/*
NAME: Simran Dhaliwal
ID: 905361068
EMAIL: sdhaliwal@ucla.edu

NAME: Jason Lai
ID: 305426666
EMAIL: jasonyslai@g.ucla.edu
*/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "ext2_fs.h"



struct ext2_super_block super_data;
struct ext2_group_desc group_data;

//name of image file that we are dealing with
char* file_name = NULL;
int fd = 0;

//
unsigned int block_size = 0; // size of block
unsigned int num_groups = 1; // number of groups
unsigned int num_of_used_inodes = 0;

/*
Wrapper function for open() syscall
Parameters:
	pathname: path to file to be opened
	flags: mode/permissions
Return value: file descriptor of opened file
*/
int Open(const char *pathname, int flags) {
	int ret = open(pathname, flags);
	if(ret < 0) {
		fprintf(stderr, "./lab3a: failed to open file '%s' -- open: %s.\n", pathname, strerror(errno));
		exit(1);
	}
	return ret;
}

/*
Wrapper function for pread() syscall
Parameters:
	fd: file descriptor to read from
	buf: buffer to read to
	count: number of bytes to read
	offset: position to start reading from
Return value: number of bytes read
*/
ssize_t Pread(int fd, void *buf, size_t count, off_t offset) {
	int ret = pread(fd, buf, count, offset);
	if (ret == -1) {
		fprintf(stderr, "./lab3a: failed to read from file descriptor %d at offset %d -- pread: %s\n", fd, (int)offset, strerror(errno));
		exit(1);
	}
	return ret;
}

/*

*/
void report_and_read_superblock(void) {
	Pread(fd, &super_data, sizeof(super_data), 1024);
	block_size = 1024 << super_data.s_log_block_size;
	fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", super_data.s_blocks_count, super_data.s_inodes_count, block_size,
	super_data.s_inode_size, super_data.s_blocks_per_group, super_data.s_inodes_per_group, super_data.s_first_ino);
}

/*

*/
void report_and_read_group(void) {
	Pread(fd, &group_data, sizeof(group_data), 1024 + block_size);

	unsigned int total_blocks_in_group;
	if (super_data.s_blocks_count < super_data.s_blocks_per_group) {
		total_blocks_in_group = super_data.s_blocks_count;
	}
	else {
		total_blocks_in_group = super_data.s_blocks_per_group;
	}

	fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", 0, total_blocks_in_group, super_data.s_inodes_per_group / num_groups, 
	group_data.bg_free_blocks_count, group_data.bg_free_inodes_count, group_data.bg_block_bitmap, group_data.bg_inode_bitmap, 
	group_data.bg_inode_table);
}

/*

*/
void report_and_read_free_blocks(void) {
	__u8* bitmap = malloc(block_size); //make enough room to read the bit map, size of one block
	Pread(fd, bitmap, block_size, 1024 + ((group_data.bg_block_bitmap-1) * block_size)); //read the bitmap, offset block number is found in group_data.bg_block_bitmap
	unsigned int i = 0, j = 0;		
	unsigned int free_block_number = 1;
	for (; i < block_size; i++) {	//for every byte read
		char byte = bitmap[i];
		for (j = 0; j < 8; j++) { //look at every bit in that byte
			if ((byte & (1  << j)) == 0) { //if the bit is off, the block is free and report, Cited in readme
				fprintf(stdout, "BFREE,%d\n", free_block_number);
			}
			
			free_block_number++;
		}
	}
	
	free(bitmap);
}

/*

*/
void report_and_read_free_inodes(void) {
	__u8* inodes_map = malloc(block_size);
	Pread(fd, inodes_map, block_size, 1024 + ((group_data.bg_inode_bitmap - 1) * block_size));
	unsigned int i = 0, j = 0;
	unsigned int free_inode_number = 1;
	for (; i < block_size; i++) {     //for every byte read
		char byte = inodes_map[i];
		for (j = 0; j < 8; j++){ //look at every bit in that byte
			if ((byte & (1  << j)) == 0) { //if the bit is off, the block is free and report, Cited in readme
				fprintf(stdout, "IFREE,%d\n", free_inode_number);
			}
			free_inode_number++;
		}
	}

	free(inodes_map);
}

/*

*/
void report_and_read_dir(struct ext2_inode inode_data, int inode_num) {
	struct ext2_dir_entry dir_data;
	unsigned int offset = 0;
	unsigned int i = 0;
	for (; i < EXT2_NDIR_BLOCKS; i++) {
		__u32 block_ptr = inode_data.i_block[i];
		if (block_ptr != 0) {
			while (offset < block_size) {
				unsigned int dir_offset = block_ptr * block_size + offset;
				Pread(fd, &dir_data, sizeof(dir_data), dir_offset);
				if (dir_data.inode != 0) {
					fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n", inode_num, offset, dir_data.inode, dir_data.rec_len,
					dir_data.name_len, dir_data.name);
				}
				offset += dir_data.rec_len;
			}
		}
	}
}

/*

*/
void report_and_read_indirect_block_refs(int inode_num, int logical_offset, int level, int block_num) {
	__u32 *blocks = malloc(block_size * sizeof(__u32));
	unsigned int offset = block_size * block_num;
	unsigned int i = 0;
	Pread(fd, blocks, block_size, offset);
	for (; i < block_size / 4; i++) {
		if (blocks[i] != 0) {
			fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", inode_num, level, logical_offset + i, block_num, blocks[i]);
			if (level == 2 || level == 3) {
				report_and_read_indirect_block_refs(inode_num, logical_offset, level - 1, blocks[i]);
			}
		}
	}
}


void time_helper(char* time, __u32  unix_time) {
	time_t b = unix_time;
	struct tm* a = gmtime(&b);
	strftime(time, 100, "%D %T", a);
}

/*

*/
void report_and_read_inodes(void) {
	unsigned int i = 1;
	unsigned int base_offset = 1024 + ((group_data.bg_inode_table-1) * block_size);

	for (; i <= super_data.s_inodes_count; i++) {
		struct ext2_inode inode_data;
		unsigned int offset = base_offset + ((i-1)*sizeof(inode_data));
		char file_type = '?';
		Pread(fd, &inode_data, sizeof(inode_data), offset);
		if (inode_data.i_mode != 0 && inode_data.i_links_count != 0) {	
			if (inode_data.i_mode & S_IFDIR)
				file_type = 'd';
			else if ((S_IFMT & inode_data.i_mode) ==  S_IFLNK)
				file_type = 's';
			if ((inode_data.i_mode & S_IFMT) ==  S_IFREG)
				file_type = 'f';

			//getting times
			char last[30], mod[30], access[30];
			time_helper(last, inode_data.i_ctime);
			time_helper(mod, inode_data.i_mtime);
			time_helper(access, inode_data.i_atime);

			fprintf(stdout, "INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d",i, file_type, inode_data.i_mode & 0xFFF, inode_data.i_uid, inode_data.i_gid,
			inode_data.i_links_count, last, mod, access, inode_data.i_size, inode_data.i_blocks);
			if (file_type == 'd' || file_type == 'f') {
				int j = 0;
				for (; j < 15; j++) {
					fprintf(stdout, ",%d", inode_data.i_block[j]);
				}
				fprintf(stdout, "\n");
			}
			else { //checking if we need to print out symbloic links
				if (inode_data.i_size > 60) {	//file len is greater than 60 print that baby out
					int j = 0;
					for (; j < 15; j++) {
						fprintf(stdout, ",%d", inode_data.i_block[j]);
					}
				}

				fprintf(stdout, "\n");
			}

			if (file_type == 'd') {
				report_and_read_dir(inode_data, i);
			}	

			if (inode_data.i_block[12] != 0) {
				report_and_read_indirect_block_refs(i, 12, 1, inode_data.i_block[12]);
			}

			if (inode_data.i_block[13] != 0) {
				report_and_read_indirect_block_refs(i, 268, 2, inode_data.i_block[13]);
			}

			if (inode_data.i_block[14] != 0) {
				report_and_read_indirect_block_refs(i, 65804, 3, inode_data.i_block[14]);
			}
		}
		
	}
	
}

int main(int argc, char ** argv) {
	
	if(argc != 2) {
		fprintf(stderr, "In correct number of arguments.\n");
		exit(1);
	}
	
	file_name = argv[1]; //getting image file from the input
	fd = Open(file_name, O_RDONLY);
	
	report_and_read_superblock();
	report_and_read_group();
	report_and_read_free_blocks();	
	report_and_read_free_inodes();
	report_and_read_inodes();

	exit(0);
}
