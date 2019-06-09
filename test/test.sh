#!/bin/bash

rm report.txt -f

for i in {1..20}
do
	echo "Case #$i Running"
	timeout 30s ./main <$i.sql >$i.ans
	code=$?
	if [ $code -eq 124 ] ; then
		echo "Case #$i: Time Limit Exceeded" >>report.txt
	elif [ $code -ne 0 ] ; then
		echo "Case #$i: Runtime Error" >>report.txt
	else
		echo "Case #$i Comparing..."
		./diff $i.ans $i.out -bB >/dev/null
		if [ $? -ne 0 ] ; then
			echo "Case #$i: Wrong Answer" >>report.txt
		else
			echo "Case #$i: Accepted" >>report.txt
			rm $i.ans
		fi
	fi
done
