class TabGroup {
    // element is the DOMelement that will hold the tabgroup and tab-buttons
    constructor(element) {
        this.element = element;

        var tabgroup = document.createElement("div");
        tabgroup.className = "brewtabs-tabgroup";
        

        var tabs = document.createElement("div");
        tabs.className = "brewtabs-tabs";
        tabgroup.appendChild(tabs);
        
        this.element.appendChild(tabgroup);

        this.DOMtabGroup = tabgroup;
        this.DOMtabs = tabs;
        this.tabArray = [];

        this.currentActive = undefined;
    }

    addTab(tab, OnClick) {
        this.tabArray.push(tab);
        var tabbutton = document.createElement("div");
        tabbutton.className = "brewtabs-tab";
        tabbutton.onclick = OnClick;
        tab.tabButton = tabbutton;

        var tabtitle = document.createElement("span");
        tabtitle.className = "brewtabs-tab-title";
        tabtitle.innerHTML = tab.title;
        tab.tabButton.appendChild(tabtitle);

        this.currentActive = this.tabArray.length - 1;

        tab.tabGroup = this;
        tab.tabArrayIndex = this.tabArray.length - 1;

        this.DOMtabs.appendChild(tab.tabButton)
    }

    removeTab(index) {
        this.tabArray[index].close();
    }

    clearTabs() {
        while(this.tabArray.length) {
            this.tabArray[0].close();
        }

        this.tabArray = [];
        this.currentActive = undefined;
    }

    moveRight() {
        if (!this.currentActive) 
            return;
        if (this.currentActive >= this.tabArray.length - 1)
            return;
        else {
            this.currentActive += 1;
            this.tabArray[this.currentActive].activate();
        }
    }

    moveLeft() {
        if (!this.currentActive) 
            return;
        if (this.currentActive === 0)
            return;
        else {
            this.currentActive -= 1;
            this.tabArray[this.currentActive].activate();
        }
    }

    closeTab(index) {
        if (this.currentActive > index) {
            this.currentActive--;
        }
        this.DOMtabs.removeChild(this.tabArray[index].tabButton)
        this.tabArray.splice(index, 1);
        this.updateIndices();

        if (this.tabArray.length !== 0) {
            this.tabArray[this.currentActive].activate();
        }
    }

    activeTab() {
        return this.tabArray(this.currentActive);
    }

    deactivateAllExcept(index) {
        for (let i = 0; i < this.tabArray.length; i++) {
            if (index === i)
                continue;
            this.tabArray[i].deactivate();
        }
        this.currentActive = index;
    }

    activatePressed(node) {
        let index = indexOfNode(node);
        this.tabArray[index].activate();
    }

    updateIndices() {
        for (let i = 0; i < this.tabArray.length; i++) {
            this.tabArray[i].tabArrayIndex = i;
        }
    }
}

class Tab {
    // Tab functionality not with the traditional meaning.
    // This is used to change an DOMtabGroup's contents by pressing a button
    // somewhat like VSCode tabs. You will be able to alternate between open tabs.
    // Once you create a Tab, you will should not be able to make any changes to the element
    // you want to change.
    // In order to start creating Tabs you will have to have created a TabGroup first.

    // element is the DOMelement that will change when alternating between tabs
    constructor(element, title, closable = true) {
        this.element = element;
        this.innerHTML = element.innerHTML;
        this.title = title;
        this.active = false;

        this.closable = closable;

        this.tabButton = undefined;
        this.tabGroup = undefined;
        this.tabArrayIndex = undefined;
    }

    activate() {
        this.active = true;
        this.tabButton.classList.add("active")
        this.element.innerHTML = this.innerHTML;

        this.tabGroup.deactivateAllExcept(this.tabArrayIndex);
    }

    deactivate() {
        this.active = false;
        this.tabButton.classList.remove("active")
    }

    close() {
        this.tabGroup.closeTab(this.tabArrayIndex);
    }
}

function indexOfNode(node) {
    let index;
    let sibling;

    index = 0;
    while ( sibling = node.previousSibling ) {
        node = sibling;
        ++index;
    }

    return index;
}

module.exports.TabGroup = TabGroup;
module.exports.Tab = Tab;