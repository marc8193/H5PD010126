let rec reverse_list list =
    seq {
        match list with
        | [] -> ()
        | head::tail ->
            yield! reverse_list tail
            yield head
    }

let liste = [1; 2; 3; 4]
for x in reverse_list liste do
    printfn "%d" x