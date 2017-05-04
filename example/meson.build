# Attention, emacs, please use -*- mode: python -*-
# (even though this isn't actually Python code)

examples = [ 'hello' ]

foreach ex : examples
    executable('nohello', ex + '.c',
               include_directories: include_dirs,
               link_with: [ libfuse ],
               install: false)
endforeach

# TODO: Link passthrough_fh with ulockmgr if available
