#!/bin/bash

## check argc
if [ "$#" -ne 1 ];then
    echo "usage} $0 modify_list.file"
	echo "env(MY_POD_NAME/MY_STATEFUL_SET) must be exist"
	echo "if [\$MY_INDEX=2] then [\$MY_INDEX+7000=>7002 \$MY_INDEX+0x700A=>0x700D \$MY_INDEX+EMGC_=EMGC_2]"
    exit
fi

## check file exist
LIST=$1
CHK_FILES=`ls $LIST`
if [[ -z $CHK_FILES ]]; then
	echo "LIST($LIST) not exist"
	exit
fi

## check env MY_POD_NAME exist
MY_POD_NAME=`printenv MY_POD_NAME`
if [[ -z $MY_POD_NAME ]]; then
	echo "MY_POD_NAME=(null)"
	exit
else
	echo "==> MY_POD_NAME=($MY_POD_NAME)"
fi

## check env MY_STATEFUL_SET exist
MY_STATEFUL_SET=`printenv MY_STATEFUL_SET`
if [[ -z $MY_STATEFUL_SET ]]; then
	echo "MY_STATEFUL_SET=(null)"
	exit
else
	echo "==> MY_STATEFUL_SET=($MY_STATEFUL_SET)"
fi

## get MY_INDEX val 
MY_INDEX=`printf '%s' "${MY_POD_NAME//$MY_STATEFUL_SET-/}"`
let "MY_INDEX += 1"
echo "==> MY_INDEX=($MY_INDEX)"

function strcmp ()
{
    if [ "$1" = "$2" ]; then
		echo 0;
    elif [ "${1}" '<' "${2}" ]; then
		echo -1;
	else
    	echo 1
	fi
}
function strncmp() {
    if [ -z "${3}" -o "${3}" -le "0" ]; then
       return 0
    fi
   
    if [ ${3} -ge ${#1} -a ${3} -ge ${#2} ]; then
       strcmp "$1" "$2"
       return $?
    else
       s1=${1:0:$3}
       s2=${2:0:$3}
       strcmp $s1 $s2
       return $?
    fi
}
function check_digit() {
	DIGIT='^[0-9]+$'
	if ! [[ $1 =~ $DIGIT ]] ; then
		echo -1;
	else
		echo 0;
	fi
}

function replace_elem() {
	STR_A=$1
	STR_B="\$MY_INDEX+"
	RES=$(strncmp $STR_A $STR_B 10)

	REMAIN_STR=`printf '%s' "${STR_A//$STR_B/}"`
	IS_DIGIT=$(check_digit $REMAIN_STR)
	IS_HEX=$(strncmp $REMAIN_STR 0x 2)

	if [ "$RES" = "0" ];then
		if [ "$IS_DIGIT" = "0" ];then
			NEW_VAL=`printf '%d' "$((REMAIN_STR + MY_INDEX))"`
			echo "$NEW_VAL"
		elif [ "$IS_HEX" = "0" ];then
			NEW_VAL=`printf '%X' "$((REMAIN_STR + MY_INDEX))"`
			echo "0x$NEW_VAL"
		else
			echo "$REMAIN_STR$MY_INDEX"
		fi
	else
		echo "$STR_A"
	fi
}

function replace_content() {
	rm -f "./$1.tmp"
	touch "./$1.tmp"
	cat $1 | while read line
	do
		str_array=($line); count=0
		for i in ${str_array[@]}
		do
			str_array[$count]=$(replace_elem $i)
			let "count += 1"
		done
	echo "${str_array[*]}" >> "$1.tmp"
	done
	mv -f "./$1.tmp" "./$1"
}

## read file list
cat $LIST | while read file
do
	replace_content $file
	sed -i -e "s/\$MY_POD_NAME/$MY_POD_NAME/g" $file
	sed -i -e "s/\$MY_STATEFUL_SET/$MY_STATEFUL_SET/g" $file
done
