gcc parser.c tree_visualizer.c allocator.c interpreter.c -g -lm -lraylib

if ls pngs/*.png >/dev/null 2>&1; then
    rm pngs/*.png
fi

./a.out lisp_src.txt > lisp_parsed.txt