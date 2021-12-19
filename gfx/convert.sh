#! /bin/bash

dirs=("." "bg" "game" "rank")
gfx_dir="$(cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"
satconv_path="$gfx_dir/../../satconv"
cd_path="$gfx_dir/../cd"

for dir in "${dirs[@]}"
do
	cd "$gfx_dir/$dir"
	"$satconv_path"/satconv assets.txt
	if ls *.map >/dev/null 2>&1
	then
		mv *.map ""$cd_path"/"$dir""
	fi

	if ls *.tle >/dev/null 2>&1
	then
		mv *.tle ""$cd_path"/"$dir""
	fi

	if ls *.spr >/dev/null 2>&1
	then
		mv *.spr ""$cd_path"/"$dir""
	fi

	cd "$gfx_dir"
done

