# astags
# (c) 1979 Regents of the University of California
grep '^[a-zA-Z_].*:' $* /dev/null >/tmp/$$
ex - /tmp/$$ << 'EOF'
1,$s/:[^:]*$//
1,$s/\(.*\):\(.*\)/\2	\1	\/^\2\//
w
q
'EOF'
sort -d /tmp/$$ | uniq >! tags
rm /tmp/$$
