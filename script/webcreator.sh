# Check if root_dir is an existing directory
if [ ! -d "$1" ]; then
	echo "$0: '$1' does not exist or is not a directory."
	exit 1
fi

# Check if text_file is an existing file
if [ ! -f "$2" ]; then
	echo "$0: '$2' does not exist or is not a file."
	exit 1
fi

# Check if input text file has atleast 10000 lines
if [ $(wc -l $2 | cut -d ' ' -f 1) -lt 10000 ]; then
	echo "$0: '$2' has less than 10000 lines."
	exit 1
fi

# Check if the directory is empty, if not purge it
if [ "$(ls -A $1)" ]; then
	echo "# Warning: directory ($1) is not empty, purging..."
	rm -fr $1/*
else
	echo "# Directory ($1) is empty. Website creation begins..."
fi

# Create an temporary file with the names of all the pages
for i in $(seq 0 $(expr $3 - 1))
do
	for j in $(seq 0 $(expr $4 - 1))
	do
		echo $1"/site"$i"/page"$j"_"$RANDOM".html" | sed 's/\/\//\//g' >> /tmp/pages
	done
done

# Create /html files for each page
for i in $(seq 0 $(expr $3 - 1))
do
	mkdir $1"/"site$i
	echo "#"
	echo "# Creating web site $i..."
	for j in $(seq 0 $(expr $4 - 1))
	do
		k=$(expr $RANDOM % $(expr $(wc -l $2 | cut -d ' ' -f 1) - 2000))
		m=$(expr $(expr $RANDOM % 1000) + 1000)
		f=$(expr $(expr $4 / 2) + 1)
		q=$(expr $(expr $3 / 2) + 1)
		NAME=$(cat /tmp/pages | head -$(expr $(expr $i \* $4) + $(expr $j \+ 1)) | tail -1)
		echo "#  Creating page $NAME with $m lines starting at line $k"
		echo "<!DOCTYPE html>" > $NAME
		echo "<html>"  >> $NAME
		echo "<body>"  >> $NAME

		grep site$i /tmp/pages | grep -v $NAME | shuf | head -$f >> /tmp/links
		grep -v site$i /tmp/pages | grep -v $NAME | shuf | head -$q >> /tmp/links
		shuf -o /tmp/links /tmp/links
		step=$(expr $m / $(expr $f + $q))
		pos=0
		for n in $(cat /tmp/links)
		do
			let pos+=$step
			cat $2 | head -$(expr $k + $pos) | tail -$step >> $NAME
			echo "#   Adding link to $n"
			echo "<a href=\"$n\">$n</a>" >> $NAME
		done

		echo "</body>" >> $NAME
		echo "</html>" >> $NAME

		cat /tmp/links >> /tmp/allLinks
		sort -u -o /tmp/allLinks /tmp/allLinks
		rm -f /tmp/links
	done
done

rm -f /tmp/pages

# Check if every page has atleast one incoming link
echo "#"
if [ $(wc -l /tmp/allLinks | cut -d ' ' -f 1) = $(expr $3 \* $4) ]; then
	echo "# All pages have at least one incoming link."
else
	echo "# Not all pages have an incoming link."
fi


rm -f /tmp/allLinks

echo "#"
echo "# Done."
