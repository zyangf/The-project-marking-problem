# The-project-marking-problem

Introduction
The problem described here is a slightly anarchic variant of the system that was formerly
used by the Department to assess MSc projects (with 2 markers per student). These days,
the markers attend separate presentations.

There are S students on a course. Each student does a project, which is assessed by K
markers. There is a panel of M markers, each of whom is required to assess up to N projects.
We assume that S*K ≤ M*N. Note that there is no guarantee that all students will complete
their demos during the session even with this constraint.

Near the end of the course a session is arranged at which each student demonstrates their
project to K markers (together). The session lasts T minutes and each demonstration takes
exactly D minutes. Students enter the lab at random intervals during the session except in
the last D minutes. All markers are on duty at the beginning of the session and each remains
there until they have attended N demos or the session ends. At the end of the session all
students and markers must leave the lab. Moreover, any students and markers who are not
actively involved in a demo D minutes before the end must leave at that time, because there
would be no time to complete a demo that started later.

Whenever a student enters the lab, they start by finding K idle markers (one at a time). If
there are any markers available, they will be "grabbed" by the student. Having grabbed K
markers, the student does the demo and then leaves the lab. Each marker stays idle until
grabbed by a student, waits until other idle markers are found (during which time the marker
cannot be grabbed by other students), attends the demo and then becomes idle again,
leaving the lab after attending N demos.
