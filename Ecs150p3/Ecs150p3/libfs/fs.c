#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "disk.h"
#include "fs.h"
#define BLOCK_SIZE 4096
#define NAME_SIZE 16
#define PADDING 10
#define FATSIZE 2
#define FAT_E0C 0xE00C
#define EMPTY_REF 0x0
/* TODO: Phase 1 */
struct superblock {
	uint64_t Signature;
	uint16_t Block_Amounts;
	uint16_t Root_Dir;
	uint16_t Data_Start;
	uint16_t Data_Blocks_Amount;
	uint8_t Fat_Blocks;
}first_block;

struct root_nodes {
	char file_name[NAME_SIZE];
	uint32_t file_size;
	uint16_t index;
	char padding[10];
} root_dir[FS_FILE_MAX_COUNT];

struct fd{
	struct root_nodes* root;
	size_t offset;
} file_descriptors[FS_OPEN_MAX_COUNT];

uint16_t* fat_representation;
int fs_mount(const char *diskname)
{
	if(block_disk_open(diskname) == -1){
		// opening failed
		return -1;
	}
	// read into first block
	block_read(0,&first_block);
	// parse the signature
	char sig_parsed[8];
	for(int i = 0; i < 8; i++) {
		sig_parsed[i] = (first_block.Signature >> (i*8)) & 0xFF;
	}
	
	char check[] = "ECS150FS";
	for(int i = 0 ; i < 8 ; i++){
		if(sig_parsed[i] != check[i]){
			return -1;
		}
	}
	
	fat_representation = (uint16_t*) malloc(first_block.Data_Blocks_Amount * sizeof(uint16_t));
	// need to match the fats and put them into fat_representation
	int block_track = 1;
	int offset = 0;
	for(block_track; block_track < 1 + first_block.Fat_Blocks; block_track++){
		if(block_read(block_track,&fat_representation[offset]) == -1){
			return -1;
		}
		offset += first_block.Data_Blocks_Amount/first_block.Fat_Blocks;
	}
	block_read(block_track,root_dir);
	return 0;
	
}

int fs_umount(void)
{
	int offset = 0;
	/* TODO: Phase 1 */
	for(int i = 1 ; i < 1 + first_block.Fat_Blocks; i++){
		if(block_write(i,&fat_representation[offset]) == -1){
			return -1;
		}
		offset += first_block.Data_Blocks_Amount/first_block.Fat_Blocks;
	}
	free(fat_representation);
	// load in the root dir
	int root_location = 1 + first_block.Fat_Blocks;
	block_write(root_location,root_dir);
	block_disk_close();
}

int fs_info(void)
{
	printf("FS Info\n");
	// total blocks
	printf("total_blk_count = %d\n",first_block.Block_Amounts);
	// fat blocks
	printf("fat_blk_count = %d\n", first_block.Fat_Blocks);
	// which block is the rdir
	printf("rdir location = %d\n", first_block.Root_Dir);
	// where is data start
	printf("data_blk = %d\n",first_block.Data_Start);
	// how many data blocks there are
	printf("data_blk_count = %d\n",first_block.Block_Amounts - first_block.Fat_Blocks - 1 - 1);
	// how many are free(fat)
	int total_fat = BLOCK_SIZE/FATSIZE*first_block.Fat_Blocks;
	int free_fat = 0;
	for(int i = 0 ; i < total_fat;i++){
		if(*(fat_representation + i) == 0){
			free_fat += 1;
		}
	}
	printf("fat_free_ratio = %d", free_fat);
	printf("/%d\n",total_fat);
	// how many free rootdirs there are
	int root_dir_elements = FS_FILE_MAX_COUNT;
	int free_dir = 0;
	for(int i = 0; i < root_dir_elements;i++){
		if(root_dir[i].file_name[0] == '\0'){
			free_dir += 1;
		}
	}
	printf("rdir_free_ratio = %d",free_dir);
	printf("/%d\n",root_dir_elements);

}

int fs_create(const char *filename)
{
	/* TODO: Phase 2 */
	// first find an available root dir
	int root_dir_elements = FS_FILE_MAX_COUNT;
	for(int i = 0; i < root_dir_elements;i++){
		if(root_dir[i].file_name[0] == '\0'){
			struct root_nodes* this_root = &root_dir[i];
			strncpy(this_root->file_name,filename,sizeof(filename));
			this_root->file_size = 0;
			// need to find a fat block
			int total_fat = BLOCK_SIZE/FATSIZE*first_block.Fat_Blocks;
			for(int j = 0 ; j < total_fat;j++){
				if(*(fat_representation + j) == 0){
					// we found a free fat block that we can allocate
					this_root->index = j;
					// change the fat's value from 0 to FAT_E0C
					*(fat_representation + j) = FAT_E0C;
					return 0;
				}
			}	
		}
	}
	return -1;
}
/// @brief clear the directory, set the index to 0, set the name to empty, set size to 0
/// @param this_root 
void clear_directory (struct root_nodes* this_root){
	this_root -> index = 0;
	this_root -> file_size = 0;
	for(int i = 0 ; i < sizeof(this_root ->file_name); i++){
		(this_root -> file_name)[i] = '\000';
	}
	return;
}
// run through the fat and clear every item the fat is conencted to
void clear_fat(int head){
	int fat_location = head;
	while(1){
		uint16_t next_fat = *(fat_representation + fat_location);
		*(fat_representation + fat_location) = 0;
		if(next_fat == FAT_E0C){
			return;
		}
	}
}
int fs_delete(const char *filename)
{
	/* TODO: Phase 2 */
	// firsat find the file in the rootdir
	int root_dir_elements = FS_FILE_MAX_COUNT;
	for(int i = 0; i < root_dir_elements;i++){
		if(strncmp(root_dir[i].file_name,filename,sizeof(root_dir[i].file_name)) == 0){
			struct root_nodes* this_root = &root_dir[i];
			// first need to know fat index
			int fat_index = this_root -> index;
			// set the name to all \000
			clear_directory(this_root);
			// clear all of the linked listed fat and make them 0
			clear_fat(fat_index);
			return 0;
		}
	}
	return -1;
}

int fs_ls(void)
{
	printf("FS Ls:\n");
	int root_dir_elements = FS_FILE_MAX_COUNT;
	int found = 0;
	for(int i = 0; i < root_dir_elements;i++){
		if(root_dir[i].file_name[0] != '\0'){
			found = 1;
			printf("file: ");
			printf("%s ",root_dir[i].file_name);
			printf("size: %d ", root_dir[i].file_size);
			printf("data_blk: %d\n", root_dir[i].index);
		}
	}
	if(!found){
		return -1;
	}
	return 0;
}

int fs_open(const char *filename)
{
	/* TODO: Phase 3 */
	// not mounted
	if(first_block.Signature == 0){
		return -1;
	}
	// file name invalid
	if(strlen(filename) > NAME_SIZE || strlen(filename) == 0){
		return -1;
	}
	// check where does this filename exist in our root
	int root_dir_elements = FS_FILE_MAX_COUNT;
	int found_root = -1; // where is the root element
	for(int i = 0 ; i < root_dir_elements; i++){
		if(strcmp(root_dir[i].file_name,filename) == 0){
			found_root = i;
			break;
		}
	}
	// the filename does not exist in the root
	if(found_root == -1){
		return -1;
	}
	int found_fd = -1;
	for(int i = 0; i < FS_OPEN_MAX_COUNT; i++){
		if(file_descriptors[i].root == EMPTY_REF){
			found_fd = i;
			file_descriptors[i].root = &root_dir[found_root];
			file_descriptors[i].offset = 0;
			break;
		}
	}
	// return -1 if we dont have a single fd that is available
	// return the fd that is assigned if we can find a fd available
	return found_fd;
}

/// @brief checks for 3 things. 1: not mounted 2: oob 3: not open
/// @param fd 
/// @return 
int fd_validation(int fd){
	if(first_block.Signature == 0){
		return -1;
	}
	// out of bounds
	if(fd < 0 || fd > FS_OPEN_MAX_COUNT - 1){
		return -1;
	}
	if(file_descriptors[fd].root == EMPTY_REF){
		return -1;
	}
	return 0;
}
int fs_close(int fd)
{
	/* TODO: Phase 3 */
	// not mounted
	if(fd_validation(fd) == -1){
		return -1;
	}
	// clear out the reference and the offset
	file_descriptors[fd].root = EMPTY_REF;
	file_descriptors[fd].offset = 0;
	return 0;

}

int fs_stat(int fd)
{
	/* TODO: Phase 3 */
	if(fd_validation(fd) == -1){
		return -1;
	}
	return file_descriptors[fd].root->file_size;
}

int fs_lseek(int fd, size_t offset)
{
	/* TODO: Phase 3 */
	if(fd_validation(fd) == -1){
		return -1;
	}
	size_t max_size = fs_stat(fd);
	// offset too large
	if(offset > max_size){
		return -1;
	}
	file_descriptors[fd].offset = offset;
	return 0;
}

/// @brief find the first available fat from the fd
/// @param this_file 
/// @return dirty fat offset is where the offset in the dirty fat
uint16_t find_dirty_fat(struct fd this_file){
	uint16_t current_fat = this_file.root->index;
	while(1){
		if(fat_representation[current_fat] == FAT_E0C){
			return current_fat + first_block.Data_Start;
		}
		current_fat = fat_representation[current_fat];
	}
}

uint16_t find_new_block(){
	for(int i = 0 ; i < sizeof(fat_representation);i++){
		if(fat_representation[i] == 0){
			// need to deal with the initial blocks such as root, fat
			return i + first_block.Data_Start;
		}
	}
	return -1;
}

void* create_writeblock (void* buf, size_t count){
	void* write = malloc(BLOCK_SIZE*sizeof(char));
	memcpy(write,buf,count);
	return write;
}

int fs_write(int fd, void *buf, size_t count)
{
	int valid = fd_validation(fd);
	if(valid == -1){
		return -1;
	}
	if(buf == NULL){
		return -1;
	}
	/* TODO: Phase 4 */
	struct fd this_file = file_descriptors[fd];
	// need to find where the first block available is
	void* buf_cpy = buf;
	uint16_t first_write_fat = find_dirty_fat(this_file);
	// first deal with the dirty fat
	// how much space we have in the dirty fat block
	uint16_t left_over_offset = BLOCK_SIZE - this_file.offset%BLOCK_SIZE;
	void* dirty_block = malloc(BLOCK_SIZE*sizeof(char));
	block_read(first_write_fat,dirty_block);
	// the left over of the dirty fat block is enough
	if(left_over_offset >= count){
		// we only need to deal with the dirty fat block
		memcpy(dirty_block + this_file.offset%BLOCK_SIZE, buf_cpy, count);
		block_write(first_write_fat,dirty_block);
		// set the offset
		this_file.root->file_size += count;
		fs_lseek(fd,this_file.offset + count);
		
		return count;
	}
	int written = 0;
	memcpy(dirty_block + this_file.offset%BLOCK_SIZE, buf_cpy, left_over_offset);
	block_write(first_write_fat,dirty_block);
	buf_cpy += left_over_offset;
	written += left_over_offset;
	// keep track of how much data we need to write
	size_t leftover_count = count;
	leftover_count -= left_over_offset;
	// the most recent block that we have wrote to
	uint16_t last_block = first_write_fat;
	while(leftover_count > 0){
		uint16_t available_fat = find_new_block(); 
		if(available_fat == -1){
			// no more blocks available
			// update the offset
			this_file.root->file_size += written;
			fs_lseek(fd,this_file.offset + written);
			
			return written;
		}
		// we have a block, now write into it
		// need to account for fake fat vs real fat
		fat_representation[last_block] = available_fat - first_block.Data_Start;
		fat_representation[available_fat] = FAT_E0C;
		if(leftover_count <= BLOCK_SIZE){
			// just write to this block and then we done
			void* new_block = create_writeblock(buf_cpy,leftover_count);
			// write it onto the data block
			block_write(available_fat,new_block);
			written += leftover_count;
			this_file.root->file_size += written;
			fs_lseek(fd,this_file.offset + written);
			
			return written;
		}
		// fresh block, we need to fill all of it
		void* new_block = create_writeblock(buf_cpy,BLOCK_SIZE);
		block_write(available_fat,new_block);
		written += BLOCK_SIZE;
		// update the buffer to next location
		buf_cpy += BLOCK_SIZE;
		leftover_count -= BLOCK_SIZE;
		
	}
	
}

uint16_t find_first_read(struct fd this_file, int* offset_left) {
	uint16_t current_fat = this_file.root->index;
	int offset = this_file.offset;
	// traverse the offset
	while(offset > BLOCK_SIZE){
		offset -= BLOCK_SIZE;
		current_fat = fat_representation[current_fat];
	}
	*offset_left = offset;
	return current_fat + first_block.Data_Start;
}

int fs_read(int fd, void *buf, size_t count)
{
	int valid = fd_validation(fd);
	if(valid == -1){
		return -1;
	}
	if(buf == NULL){
		return -1;
	}
	void *orig_buf = buf;
	/* TODO: Phase 4 */
	size_t count_left = count;
	int offset_left = 0;
	uint16_t current_fat = find_first_read(file_descriptors[fd], &offset_left);
	void* dirty_block = malloc(BLOCK_SIZE*sizeof(char));
	block_read(current_fat,dirty_block);
	// if whatever is left over of the dirty block is greater than count
	if(count < BLOCK_SIZE - offset_left){
		memcpy(buf,dirty_block + offset_left, count);
		buf = orig_buf;
		return 0;
	}
	memcpy(buf,dirty_block + offset_left, BLOCK_SIZE - offset_left);
	buf += (BLOCK_SIZE - offset_left);
	count_left -= (BLOCK_SIZE - offset_left);
	// move current fat to next
	current_fat = fat_representation[current_fat] + first_block.Data_Start;
	while(count_left > 0){
		void* new_block = malloc(BLOCK_SIZE*sizeof(char));
		block_read(current_fat,new_block);
		if(count_left <= BLOCK_SIZE){
			// read the amount we need and return
			memcpy(buf,new_block,count_left);
			buf = orig_buf;
			return 0;
		}
		// arrive here if the count that we got left > a whole block
		memcpy(buf, new_block, BLOCK_SIZE);
		buf += BLOCK_SIZE;
		count_left -= BLOCK_SIZE;
		current_fat = fat_representation[current_fat];
		if(current_fat == FAT_E0C){
			// the amount we want to read > the amount of data we actually have
			buf = orig_buf;
			return 0;
		}
	}
}

