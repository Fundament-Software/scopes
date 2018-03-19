
fn source-ui (sxitem)
    let item = (sxitem as Any)
    let T = ('typeof item)
    if (T == list)
        let loop (item) = (item as list)
        if (empty? item)
            return;
        let subitem = ((list-at item) as Syntax)
        source-ui subitem
        loop (list-next item)
    #else
        print item
    return;

source-ui
    list-load module-path
