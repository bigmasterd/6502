for d in */ ; do
    echo "building $d..."
    cd $d
    xa *
    cd ..
done
