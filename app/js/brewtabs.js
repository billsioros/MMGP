class TabGroup {
    // element is the DOMelement that will hold the tabgroup and tab-buttons
    constructor(element) {
        this.element = element;

        var tabgroup = document.createElement("div");
        tabgroup.className = "brewtabs-tabgroup";
        

        var tabs = document.createElement("span");
        tabs.className = "brewtabs-tabs";
        tabgroup.appendChild(tabs);
        
        this.element.appendChild(tabgroup);

        this.DOMtabGroup = tabgroup;
        this.DOMtabs = tabs;
        this.tabArray = [];

        this.currentActive = undefined;
        this.length = this.tabArray.length;
    }

    addTab(tab, OnClick, OnClose = undefined) {
        this.tabArray.push(tab);
        var tabbutton = document.createElement("div");
        tabbutton.className = "brewtabs-tab";
        tabbutton.onclick = OnClick;
        tabbutton.classList.add("tooltip")
        tab.tabButton = tabbutton;

        

        var tabtitle = document.createElement("span");
        tabtitle.className = "brewtabs-tab-title";
        tabtitle.innerHTML = tab.title;
        tab.tabButton.appendChild(tabtitle);

        var tabtooltiptext = document.createElement("span");
        tabtooltiptext.className = "brewtabs-tab-tootltiptext";
        tabtooltiptext.classList.add("tooltiptext");
        tabtooltiptext.innerHTML = tab.tooltip;
        tabtooltiptext.hidden = true;
        tab.tabButton.appendChild(tabtooltiptext);

        if (tab.closable) {
            var closebutton = document.createElement("button");
            closebutton.className = "brewtabs-tab-closebutton";
            closebutton.onclick = OnClose;
            closebutton.type = "button";

            let closeimg = document.createElement("img");
            closeimg.src = "../images/General/x.png";
            closeimg.className = "brewtabs-tab-closebutton-image";

            closebutton.appendChild(closeimg);

            tab.closebutton = closebutton;
            tab.tabButton.appendChild(closebutton)
        }

        this.currentActive = this.tabArray.length - 1;

        tab.tabGroup = this;
        tab.tabArrayIndex = this.tabArray.length - 1;
        this.length = this.tabArray.length;

        this.DOMtabs.appendChild(tab.tabButton);
    }

    removeTab(index) {
        this.tabArray[index].close();
    }

    clearTabs() {
        while(this.tabArray.length) {
            if (this.tabArray[0])
            this.tabArray[0].close();
        }

        this.tabArray = [];
        this.currentActive = undefined;
        this.length = 0;
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
        if (this.currentActive > index || index === this.tabArray.length - 1) {
            this.currentActive--;
        }
        this.DOMtabs.removeChild(this.tabArray[index].tabButton)
        this.tabArray.splice(index, 1);
        this.updateIndices();

        this.length = this.tabArray.length;
    }

    activeTab() {
        return this.tabArray[this.currentActive];
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
        return this.tabArray[index];
    }

    closePressed(node) {
        let index = indexOfNode(node.parentNode);
        this.tabArray[index].close();
        return this.tabArray[index];
    }

    getPressed(node) {
        let index = indexOfNode(node);
        return this.tabArray[index];
    }

    getClosed(node) {
        let index = indexOfNode(node.parentNode);
        return this.tabArray[index];
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
    constructor(elements, title, type, closable = true, tooltip = "") {
        this.elements = elements;
        this.innerHTMLs = [];

        elements.forEach(element => {
            this.innerHTMLs.push(element.innerHTML);
        });

        this.title = title;
        this.type = type;
        this.active = false;

        this.closable = closable;
        this.closed = false;

        if (!tooltip) {
            tooltip = "Show " + title;
        }
        this.tooltip = tooltip;

        this.tabButton = undefined;
        this.tabGroup = undefined;
        this.tabArrayIndex = undefined;
    }

    activate(change=true) {
        this.active = true;
        this.tabButton.classList.add("active")

        if(change) {
            for (let i = 0; i < this.elements.length; i++) {
                this.elements[i].innerHTML = this.innerHTMLs[i];
            }
        }

        this.tabGroup.deactivateAllExcept(this.tabArrayIndex);
    }

    deactivate() {
        this.active = false;
        this.tabButton.classList.remove("active")
    }

    close() {
        this.closed = true;
        this.tabGroup.closeTab(this.tabArrayIndex);
    }

    updateContents() {
        for (let i = 0; i < this.elements.length; i++) {
            this.innerHTMLs[i] = this.elements[i].innerHTML;
        }
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