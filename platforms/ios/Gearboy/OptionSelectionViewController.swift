import UIKit

final class OptionSelectionViewController: UITableViewController {
    private let optionTitles: [String]
    private var selectedIndex: Int
    private let onSelection: (Int) -> Void

    init(title: String, optionTitles: [String], selectedIndex: Int, onSelection: @escaping (Int) -> Void) {
        self.optionTitles = optionTitles
        self.selectedIndex = selectedIndex
        self.onSelection = onSelection
        super.init(style: .insetGrouped)
        self.title = title
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        optionTitles.count
    }

    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = UITableViewCell(style: .default, reuseIdentifier: nil)
        cell.textLabel?.text = optionTitles[indexPath.row]
        cell.accessoryType = indexPath.row == selectedIndex ? .checkmark : .none
        return cell
    }

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        selectedIndex = indexPath.row
        onSelection(selectedIndex)
        tableView.reloadData()
        navigationController?.popViewController(animated: true)
    }
}
