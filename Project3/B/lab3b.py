'''
NAME: Simran Dhaliwal
ID: 905361068
EMAIL: sdhaliwal@ucla.edu

NAME: Jason Lai
ID: 305426666
EMAIL: jasonyslai@g.ucla.edu

'''


import csv, sys
from collections import defaultdict

# This class holds all info in the super block line
class Super_Block:
	def __init__(self, total_blocks, total_inodes, starting_inode):
		self.total_blocks = total_blocks
		self.total_inodes = total_inodes
		self.starting_inode = starting_inode


# This class holds all infor for a directory entry
class Dir_Entry:
	def __init__(self, inode_num, dir_name, referenced_inode):
		self.inode_num = inode_num
		self.dir_name = dir_name
		self.referenced_inode = referenced_inode


def block_error(Error, position, block_number, inode_number):
	if position == 26:
		print(Error + ' TRIPLE INDIRECT BLOCK ' + block_number + ' IN INODE ' + inode_number + ' AT OFFSET 65804') 
	elif position == 25:
		print(Error + ' DOUBLE INDIRECT BLOCK ' + block_number + ' IN INODE ' + inode_number + ' AT OFFSET 268') 
	elif position == 24:
		print(Error + ' INDIRECT BLOCK ' + block_number + ' IN INODE ' + inode_number + ' AT OFFSET 12') 
	else:
		print(Error + ' BLOCK ' + block_number + ' IN INODE ' + inode_number + ' AT OFFSET 0') 


# This function performs block auditing
def audit_blocks(super_block, block_refs, free_blocks, inconsist):
	for block in block_refs:
		if block in free_blocks:
			print('ALLOCATED BLOCK', block, 'ON FREELIST')
			inconsist = True
		if len(block_refs[block]) > 1:
			for x in block_refs[block]:
				block_error("DUPLICATE", x[0], str(block), x[1])
				inconsist = True
	
	for block in range(1, super_block.total_blocks-1):
		if block > 8 and block not in block_refs and block not in free_blocks:
			print('UNREFERENCED BLOCK', block)
			inconsist = True


# This function performs inode auditing
def audit_inodes(super_block, used_inodes, free_inodes, inconsist):
	for inode in used_inodes:
		if inode in free_inodes:
			print('ALLOCATED INODE', inode, 'ON FREELIST')
			inconsist = True
	
	for inode in range(super_block.starting_inode, super_block.total_inodes + 1):
		if inode not in used_inodes and inode not in free_inodes:
			print('UNALLOCATED INODE', inode, 'NOT ON FREELIST')
			inconsist = True

# This function checks for incorrectly referenced inodes
def audit_directory_ref_count(inode_ref_count, dir_links, inconsist):
	for k, v in inode_ref_count.items():
		if dir_links[k] != v:
			print('INODE', k, 'HAS', dir_links[k], 'LINKS BUT LINKCOUNT IS', v)
			inconsist = True

# This function checks if the directory entries refer to valid and allocated inodes
def audit_directory_valid_inodes(super_block, dir_entries, free_inodes, inconsist):
	for entry in dir_entries:
		if entry.dir_name != '.' and entry.dir_name != '..':
			if entry.referenced_inode >= super_block.total_inodes:
				print('DIRECTORY INODE', entry.inode_num, 'NAME \'' + entry.dir_name + '\' INVALID INODE', entry.referenced_inode)
				inconsist = True
			elif entry.referenced_inode in free_inodes:
				print('DIRECTORY INODE', entry.inode_num, 'NAME \'' + entry.dir_name + '\' UNALLOCATED INODE', entry.referenced_inode)
				inconsist = True

# This function checks the validity of the implicit '.' and '..' links in the directory
def audit_directory_implicit(dir_entries, dir_parents, inconsist):
	for entry in dir_entries:
		if entry.dir_name == '.' and entry.referenced_inode != entry.inode_num:
			print('DIRECTORY INODE', entry.inode_num, 'NAME \'' + entry.dir_name + '\' LINK TO INODE',
				entry.referenced_inode, 'SHOULD BE', entry.inode_num)
			inconsist = True
		if entry.dir_name == '..' and entry.referenced_inode != dir_parents[entry.inode_num]:
			print('DIRECTORY INODE', entry.inode_num, 'NAME \'' + entry.dir_name + '\' LINK TO INODE',
				entry.referenced_inode, 'SHOULD BE', dir_parents[entry.inode_num])
			inconsist = True

def main():
	super_block = None
	free_blocks = [] 					# holds all the free block numbers
	free_inodes = [] 					# holds all the free inode numbers	
	used_inodes = [] 					# holds all inodes that are used
	dir_entries = []					# holds all the directory entries
	inode_ref_count = defaultdict(int)	# holds all the inodes' reference count
	dir_links = defaultdict(int)		# holds all the discovered links for directory
	dir_parents = defaultdict(int)		# holds all the parents of a file/directory
	dir_parents[2] = 2					# parent of top-most directory is always itself
	block_refs = defaultdict(list)

	inconsist = False

	if len(sys.argv) !=  2:
		print('Invalid number of arguments')
		sys.exit(1) 
	
	with open(sys.argv[1]) as csvfile:
		reader = csv.reader(csvfile, delimiter=',')
		for row in reader:
			if row[0] == 'BFREE':
				free_blocks.append(int(row[1]))

			if row[0] == 'IFREE':
				free_inodes.append(int(row[1]))

			if row[0] == 'SUPERBLOCK':
				super_block = Super_Block(int(row[1]), int(row[2]), int(row[7]))

			if row[0] == 'INODE':
				used_inodes.append(int(row[1]))
				inode_ref_count[int(row[1])] = int(row[6])
				if(row[2] == 's' and int(row[10]) <= 60):
					continue
				data_blocks = row[12:27]
				i = 12
				for x in data_blocks:
					if int(x) < 0 or int(x) > int(super_block.total_blocks):
						block_error("INVALID", i, x, row[1])
						inconsist = True
					elif int(x) < 4 and int(x) != 0:
						block_error("RESERVED", i, x, row[1])
						inconsist = True
					if int(x) != 0:
						block_refs[int(x)].append([i, row[1]])
					i += 1

			if row[0] == 'DIRENT':
				row[6] = row[6][1:-1]
				dir_links[int(row[3])] += 1
				dir_entries.append(Dir_Entry(int(row[1]), row[6], int(row[3])))
				if row[6] != '.' and row[6] != '..':
					dir_parents[int(row[3])] = int(row[1])

			if row[0] == "INDIRECT":
				#checking blocks in each indirect
				#row[5] is block number, and row[2] is indirectness level, row[1] is inode
				pos = 0
				if(int(row[2]) == 1):
					pos = 24
				elif (int(row[2]) == 2):
					pos = 25
				else:
					pos = 26
				if(int(row[5]) < 0 or int(row[5]) > int(super_block.total_blocks)):
					block_error("INVALID", pos, row[5], row[1])
					inconsist = True
				elif int(row[5]) != 0 and int(row[5]) < 4:
					block_error("RESERVED", pos, row[5], row[1])
					inconsist = True
				block_refs[int(row[5])].append([i, row[1]])
	
	
	audit_blocks(super_block, block_refs, free_blocks, inconsist)
	audit_inodes(super_block, free_inodes, used_inodes, inconsist)
	audit_directory_ref_count(inode_ref_count, dir_links, inconsist)
	audit_directory_valid_inodes(super_block, dir_entries, free_inodes, inconsist)
	audit_directory_implicit(dir_entries, dir_parents, inconsist)

	if inconsist:
		sys.exit(2)
	else:
		sys.exit(0)


if __name__ == '__main__':
	main()
 
