# ensure this script is being sourced:
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
	echo "Please source this script instead of running it."
	exit 1
fi

# find the relative path to the common directory
# This is the beginning part of the path to the symlink script
common_dir=${${0}%/$(basename ${0})}

# symlink everythingeitive path 
# n the common directory to the current directory
# except for this script
for file in $(ls ${common_dir} | grep -v $(basename ${0})); do
	ln -fs ${common_dir}/${file} ${file}
done
