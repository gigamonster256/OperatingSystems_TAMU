# ensure this script is being sourced:
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
	echo "Please source this script instead of running it."
	exit 1
fi

# find the common directory
# this is the directory containing this script
common_dir=$(dirname $(readlink -f ${0}))

# symlink everything in the common directory to the current directory
# except for this script
for file in $(ls ${common_dir} | grep -v $(basename ${0})); do
	ln -fs ${common_dir}/${file} ${file}
done
