A placeholder for the real libicpf library. Before building CH
you should replace the src/libicpf directory with the real libicpf
library, so the solution would load the real libicpf project and not
the fake one.

FAQ:
Q) Why is this subproject empty ?
A) Because it only the placeholder - you should replace the placeholder
   with the real libicpf library and then build the entire solution.
   
Q) Why not just include real libicpf in the solution instead of this placeholder?
A) libicpf is a separate project and is maintained independently of CH, so it can't
   be included as a full library here (it would be tempting to make changes to the
   libicpf through the CH project instead of making those changes in the separate
   project). Of course this library is being used in my other projects.
   
If you have more questions regarding this matter or would like to suggest some
other solution - please contact the author of this project at:
ixen@copyhandler.com.