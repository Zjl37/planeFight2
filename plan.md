## Plan

| Item | Description | Achieve time |
| :--- | :---------- | :----------- |
| Non-blocking recieve event | The current Winsock IO mode blocks the program when recieving messages, during which time players cannot perform operations like marking on the map. | ?* |
| Keyboard Event | Although mouse interaction support is a feature, most process can't be done with keyboard only. What's more, a console isn't designed to receive mouse event. The goal is to make all process possible to be done using only keyboard. | ?* |

New idea? Raise an issue!

## Achieved

| Item | Description | Version |
| :--- | :---------- | :------ |
| Changeable map size | Map size became a gamerule option. | 2.2 |
| Multi-language Support | All texts as part of UI moved out of source code. Program reads language file on initiation. | 2.1 |

---

*As I'm currently a high school student, I don't always have access to computer, so issues may not be handled in time.