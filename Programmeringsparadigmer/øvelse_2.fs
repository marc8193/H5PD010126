let rec count_elements list =
    match list with
    | [] -> 0
    | _::tail -> 1 + count_elements tail

let liste = [1; 2; 3; 4; 5]
printfn "Output: %d" (count_elements liste)